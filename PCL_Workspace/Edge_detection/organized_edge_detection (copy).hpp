/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2012-, Open Perception, Inc.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef IMPL_ORGANIZED_EDGE_DETECTION_H_
#define IMPL_ORGANIZED_EDGE_DETECTION_H_

#include <pcl/2d/edge.h>
#include "organized_edge_detection.h"
#include <pcl/console/print.h>
#include <pcl/console/time.h>

/**
 *  Directions: 1 2 3
 *              0 x 4
 *              7 6 5
 * e.g. direction y means we came from pixel with label y to the center pixel x
 */
//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointLT> void
pcl::OrganizedEdgeBase<PointT, PointLT>::compute (pcl::PointCloud<PointLT>& labels, std::vector<pcl::PointIndices>& label_indices) const
{
  pcl::Label invalid_pt;
  invalid_pt.label = unsigned (0);
  labels.points.resize (input_->points.size (), invalid_pt);
  labels.width = input_->width;
  labels.height = input_->height;
  
  extractEdges (labels);

  assignLabelIndices (labels, label_indices);
}

//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointLT> void
pcl::OrganizedEdgeBase<PointT, PointLT>::assignLabelIndices (pcl::PointCloud<PointLT>& labels, std::vector<pcl::PointIndices>& label_indices) const
{
  const unsigned invalid_label = unsigned (0);
  label_indices.resize (num_of_edgetype_);
  for (unsigned idx = 0; idx < input_->points.size (); idx++)
  {
    if (labels[idx].label != invalid_label)
    {
      for (int edge_type = 0; edge_type < num_of_edgetype_; edge_type++)
      {
        if ((labels[idx].label >> edge_type) & 1)
          label_indices[edge_type].indices.push_back (idx);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointLT> void
pcl::OrganizedEdgeBase<PointT, PointLT>::extractEdges (pcl::PointCloud<PointLT>& labels) const
{
  if ((detecting_edge_types_ & EDGELABEL_NAN_BOUNDARY) || (detecting_edge_types_ & EDGELABEL_OCCLUDING) || (detecting_edge_types_ & EDGELABEL_OCCLUDED))
  {
    // Fill lookup table for next points to visit
    const int num_of_ngbr = 8;
    Neighbor directions [num_of_ngbr] = {Neighbor(-1, 0, -1),
      Neighbor(-1, -1, -labels.width - 1), 
      Neighbor( 0, -1, -labels.width    ),
      Neighbor( 1, -1, -labels.width + 1),
      Neighbor( 1,  0,                 1),
      Neighbor( 1,  1,  labels.width + 1),
      Neighbor( 0,  1,  labels.width    ),
      Neighbor(-1,  1,  labels.width - 1)};

    for (int row = 1; row < int(input_->height) - 1; row++)
    {
      for (int col = 1; col < int(input_->width) - 1; col++)
      {
        int curr_idx = row*int(input_->width) + col;
        if (!pcl_isfinite (input_->points[curr_idx].z))
          continue;

        float curr_depth = fabsf (input_->points[curr_idx].z);

        // Calculate depth distances between current point and neighboring points
        std::vector<float> nghr_dist;
        nghr_dist.resize (8);
        bool found_invalid_neighbor = false;
        for (int d_idx = 0; d_idx < num_of_ngbr; d_idx++)
        {
          int nghr_idx = curr_idx + directions[d_idx].d_index;
          assert (nghr_idx >= 0 && nghr_idx < input_->points.size ());
          if (!pcl_isfinite (input_->points[nghr_idx].z))
          {
            found_invalid_neighbor = true;
            break;
          }
          nghr_dist[d_idx] = curr_depth - fabsf (input_->points[nghr_idx].z);
        }

        if (!found_invalid_neighbor)
        {
          // Every neighboring points are valid
          std::vector<float>::iterator min_itr = std::min_element (nghr_dist.begin (), nghr_dist.end ());
          std::vector<float>::iterator max_itr = std::max_element (nghr_dist.begin (), nghr_dist.end ());
          float nghr_dist_min = *min_itr;
          float nghr_dist_max = *max_itr;
          float dist_dominant = fabsf (nghr_dist_min) > fabsf (nghr_dist_max) ? nghr_dist_min : nghr_dist_max;
          if (fabsf (dist_dominant) > th_depth_discon_*fabsf (curr_depth))
          {
            // Found a depth discontinuity
            if (dist_dominant > 0.f)
            {
              if (detecting_edge_types_ & EDGELABEL_OCCLUDED)
                labels[curr_idx].label |= EDGELABEL_OCCLUDED;
            }
            else
            {
              if (detecting_edge_types_ & EDGELABEL_OCCLUDING)
                labels[curr_idx].label |= EDGELABEL_OCCLUDING;
            }
          }
        }
        else
        {
          // Some neighboring points are not valid (nan points)
          // Search for corresponding point across invalid points
          // Search direction is determined by nan point locations with respect to current point
          int dx = 0;
          int dy = 0;
          int num_of_invalid_pt = 0;
          for (int d_idx = 0; d_idx < num_of_ngbr; d_idx++)
          {
            int nghr_idx = curr_idx + directions[d_idx].d_index;
            assert (nghr_idx >= 0 && nghr_idx < input_->points.size ());
            if (!pcl_isfinite (input_->points[nghr_idx].z))
            {
              dx += directions[d_idx].d_x;
              dy += directions[d_idx].d_y;
              num_of_invalid_pt++;
            }
          }

          // Search directions
          assert (num_of_invalid_pt > 0);
          float f_dx = static_cast<float> (dx) / static_cast<float> (num_of_invalid_pt);
          float f_dy = static_cast<float> (dy) / static_cast<float> (num_of_invalid_pt);

          // Search for corresponding point across invalid points
          float corr_depth = std::numeric_limits<float>::quiet_NaN ();
          for (int s_idx = 1; s_idx < max_search_neighbors_; s_idx++)
          {
            int s_row = row + static_cast<int> (std::floor (f_dy*static_cast<float> (s_idx)));
            int s_col = col + static_cast<int> (std::floor (f_dx*static_cast<float> (s_idx)));

            if (s_row < 0 || s_row >= int(input_->height) || s_col < 0 || s_col >= int(input_->width))
              break;

            if (pcl_isfinite (input_->points[s_row*int(input_->width)+s_col].z))
            {
              corr_depth = fabsf (input_->points[s_row*int(input_->width)+s_col].z);
              break;
            }
          }

          if (!pcl_isnan (corr_depth))
          {
            // Found a corresponding point
            float dist = curr_depth - corr_depth;
            if (fabsf (dist) > th_depth_discon_*fabsf (curr_depth))
            {
              // Found a depth discontinuity
              if (dist > 0.f)
              {
                if (detecting_edge_types_ & EDGELABEL_OCCLUDED)
                  labels[curr_idx].label |= EDGELABEL_OCCLUDED;
              }
              else
              {
                if (detecting_edge_types_ & EDGELABEL_OCCLUDING)
                  labels[curr_idx].label |= EDGELABEL_OCCLUDING;
              }
            }
          } 
          else
          {
            // Not found a corresponding point, just nan boundary edge
            if (detecting_edge_types_ & EDGELABEL_NAN_BOUNDARY)
              labels[curr_idx].label |= EDGELABEL_NAN_BOUNDARY;
          }
        }
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointLT> void
pcl::OrganizedEdgeFromRGB<PointT, PointLT>::compute (pcl::PointCloud<PointLT>& labels, std::vector<pcl::PointIndices>& label_indices) const
{
  pcl::Label invalid_pt;
  invalid_pt.label = unsigned (0);
  labels.points.resize (input_->points.size (), invalid_pt);
  labels.width = input_->width;
  labels.height = input_->height;

  OrganizedEdgeBase<PointT, PointLT>::extractEdges (labels);
  extractEdges (labels);

  this->assignLabelIndices (labels, label_indices);
}

//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointLT> void
pcl::OrganizedEdgeFromRGB<PointT, PointLT>::extractEdges (pcl::PointCloud<PointLT>& labels) const
{
  if ((detecting_edge_types_ & EDGELABEL_RGB_CANNY))
  {
    pcl::PointCloud<PointXYZI>::Ptr gray (new pcl::PointCloud<PointXYZI>);
    gray->width = input_->width;
    gray->height = input_->height;
    gray->resize (input_->height*input_->width);

    for (size_t i = 0; i < input_->size (); ++i)
      (*gray)[i].intensity = float (((*input_)[i].r + (*input_)[i].g + (*input_)[i].b) / 3);

    pcl::PointCloud<pcl::PointXYZIEdge> img_edge_rgb;
    pcl::Edge<PointXYZI, pcl::PointXYZIEdge> edge;
    edge.setInputCloud (gray);
    edge.setHysteresisThresholdLow (th_rgb_canny_low_);
    edge.setHysteresisThresholdHigh (th_rgb_canny_high_);
    edge.detectEdgeCanny (img_edge_rgb);
    
    for (uint32_t row=0; row<labels.height; row++)
    {
      for (uint32_t col=0; col<labels.width; col++)
      {
        if (img_edge_rgb (col, row).magnitude == 255.f)
          labels[row * labels.width + col].label |= EDGELABEL_RGB_CANNY;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointNT, typename PointLT> void
pcl::OrganizedEdgeFromNormals<PointT, PointNT, PointLT>::compute (pcl::PointCloud<PointLT>& labels, std::vector<pcl::PointIndices>& label_indices) const
{
  pcl::Label invalid_pt;
  invalid_pt.label = unsigned (0);
  labels.points.resize (input_->points.size (), invalid_pt);
  labels.width = input_->width;
  labels.height = input_->height;
  
  OrganizedEdgeBase<PointT, PointLT>::extractEdges (labels);
  extractEdges (labels);

  this->assignLabelIndices (labels, label_indices);
}

//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointNT, typename PointLT> void
pcl::OrganizedEdgeFromNormals<PointT, PointNT, PointLT>::extractEdges (pcl::PointCloud<PointLT>& labels) const
{


  //Method I_0
  if ((detecting_edge_types_ & EDGELABEL_HIGH_CURVATURE))
  {
    pcl::PointCloud<PointXYZI>::Ptr gray (new pcl::PointCloud<PointXYZI>);
    gray->width = input_->width;
    gray->height = input_->height;
    gray->resize (input_->height*input_->width);


      Eigen::Vector3f normal_centroid = Eigen::Vector3f::Zero ();
      int cp = 0;

      for (size_t i = 0; i < input_->size (); ++i)
      {
        if (!pcl_isfinite (normals_->points[i].normal[0])
            ||
            !pcl_isfinite (normals_->points[i].normal[1])
            ||
            !pcl_isfinite (normals_->points[i].normal[2]))
          continue;
        normal_centroid += normals_->points[i].getNormalVector3fMap ();

        //normal_centroid[0] += normals_->points[i].normal_x;
        //normal_centroid[1] += normals_->points[i].normal_y;
        //normal_centroid[2] += normals_->points[i].normal_z;
        cp++;
      }
    
    normal_centroid /= static_cast<float> (cp);

    normal_centroid.normalize (); 



    //Eigen::Vector3f x_vector = Eigen::Vector3f::UnitX ();
    //Eigen::Vector3f y_vector = Eigen::Vector3f::UnitY ();
    //Eigen::Vector3f z_vector = Eigen::Vector3f::UnitZ ();

    for (size_t i = 0; i < input_->size (); ++i)
        {
        Eigen::Vector3f plane_normal;
        plane_normal[0] = normals_->points[i].normal_x;
        plane_normal[1] = normals_->points[i].normal_y;
        plane_normal[2] = normals_->points[i].normal_z;

        plane_normal.normalize ();
        Eigen::Vector3f axis = plane_normal.cross(normal_centroid);

        (*gray)[i].intensity = float (20*axis.norm());

        }
      

//(*gray)[i].intensity = float (10*(normals_->points[i].normal_x + normals_->points[i].normal_y + normals_->points[i].normal_z)+0.0001*(((*input_)[i].r + (*input_)[i].g + (*input_)[i].b) / 3)+200*normals_->points[i].curvature);

//(*gray)[i].intensity = float (200*normals_->points[i].curvature);


    pcl::PointCloud<pcl::PointXYZIEdge> img_edge_rgb;
    pcl::Edge<PointXYZI, pcl::PointXYZIEdge> edge;
    edge.setInputCloud (gray);
    double th_rgb_canny_low_ =4.8;
    double th_rgb_canny_high_ =7.0;
    edge.setHysteresisThresholdLow (th_rgb_canny_low_);
    edge.setHysteresisThresholdHigh (th_rgb_canny_high_);


    edge.detectEdgeCanny (img_edge_rgb);

    for (uint32_t row=0; row<labels.height; row++)
    {
      for (uint32_t col=0; col<labels.width; col++)
      {
        if (img_edge_rgb (col, row).magnitude == 255.f)
          labels[row * labels.width + col].label |= EDGELABEL_HIGH_CURVATURE;
      }
    }
  }


/*
    edge.detectEdgeSobel (img_edge_rgb);
    
    for (uint32_t row=0; row<labels.height; row++)
    {
      for (uint32_t col=0; col<labels.width; col++)
      {
        if (img_edge_rgb (col, row).magnitude > 3.5f)
          labels[row * labels.width + col].label |= EDGELABEL_HIGH_CURVATURE;
      }
    }
  }
*/  

/*
    edge.detectEdgePrewitt (img_edge_rgb);
    
    for (uint32_t row=0; row<labels.height; row++)
    {
      for (uint32_t col=0; col<labels.width; col++)
      {
        if (img_edge_rgb (col, row).magnitude > 3.0f)
          labels[row * labels.width + col].label |= EDGELABEL_HIGH_CURVATURE;
      }
    }
  }
*/

/*
    edge.detectEdgeRoberts (img_edge_rgb);
    
    for (uint32_t row=0; row<labels.height; row++)
    {
      for (uint32_t col=0; col<labels.width; col++)
      {
        if (img_edge_rgb (col, row).magnitude > 0.7f)
          labels[row * labels.width + col].label |= EDGELABEL_HIGH_CURVATURE;
      }
    }
  }
*/




/*
  //Method I_1
  if ((detecting_edge_types_ & EDGELABEL_HIGH_CURVATURE))
  {
    pcl::PointCloud<PointXYZI>::Ptr gray (new pcl::PointCloud<PointXYZI>);
    gray->width = input_->width;
    gray->height = input_->height;
    gray->resize (input_->height*input_->width);

    for (size_t i = 0; i < input_->size (); ++i)
      (*gray)[i].intensity = float (10*(normals_->points[i].normal_x + normals_->points[i].normal_y + normals_->points[i].normal_z));

    pcl::PointCloud<pcl::PointXYZIEdge> img_edge_rgb;
    pcl::Edge<PointXYZI, pcl::PointXYZIEdge> edge;
    edge.setInputCloud (gray);
    double th_rgb_canny_low_ =3;
    double th_rgb_canny_high_ =15.0;
    edge.setHysteresisThresholdLow (th_rgb_canny_low_);
    edge.setHysteresisThresholdHigh (th_rgb_canny_high_);
    edge.detectEdgeCanny (img_edge_rgb);
    
    for (uint32_t row=0; row<labels.height; row++)
    {
      for (uint32_t col=0; col<labels.width; col++)
      {
        if (img_edge_rgb (col, row).magnitude == 255.f)
          labels[row * labels.width + col].label |= EDGELABEL_HIGH_CURVATURE;
      }
    }
  }
*/



/*
  // METHOD II    

  if ((detecting_edge_types_ & EDGELABEL_HIGH_CURVATURE))
  {

    const int num_of_ngbr = 8;
    Neighbor directions [num_of_ngbr] = {Neighbor(-1, 0, -1),
      Neighbor(-1, -1, -labels.width - 1), 
      Neighbor( 0, -1, -labels.width    ),
      Neighbor( 1, -1, -labels.width + 1),
      Neighbor( 1,  0,                 1),
      Neighbor( 1,  1,  labels.width + 1),
      Neighbor( 0,  1,  labels.width    ),
      Neighbor(-1,  1,  labels.width - 1)};

    for (int row = 1; row < int(input_->height) - 1; row++)
    {
      for (int col = 1; col < int(input_->width) - 1; col++)
      {
        int curr_idx = row*int(input_->width) + col;
  
        std::vector<float> nghr_dist;
        nghr_dist.resize (8);

        for (int d_idx = 0; d_idx < num_of_ngbr; d_idx++)
        {
          int nghr_idx = curr_idx + directions[d_idx].d_index;
          assert (nghr_idx >= 0 && nghr_idx < input_->points.size ());
          
          Eigen::Vector3f curr_normal;
          curr_normal[0]=normals_->points[curr_idx].normal_x;       
          curr_normal[1]=normals_->points[curr_idx].normal_y; 
          curr_normal[2]=normals_->points[curr_idx].normal_z; 

          Eigen::Vector3f nghr_normal;
          nghr_normal[0]=normals_->points[nghr_idx].normal_x;       
          nghr_normal[1]=normals_->points[nghr_idx].normal_y; 
          nghr_normal[2]=normals_->points[nghr_idx].normal_z; 

          Eigen::Vector3f curr_axis = curr_normal.cross(nghr_normal);

          double curr_rotation=asin(curr_axis.norm());

	  nghr_dist[d_idx] = curr_rotation; 		
        }

          double th_normal_discon_=0.13;
          std::vector<float>::iterator min_itr = std::min_element (nghr_dist.begin (), nghr_dist.end ());
          std::vector<float>::iterator max_itr = std::max_element (nghr_dist.begin (), nghr_dist.end ());
          float nghr_dist_min = *min_itr;
          float nghr_dist_max = *max_itr;
          float dist_dominant = fabsf (nghr_dist_min) > fabsf (nghr_dist_max) ? nghr_dist_min : nghr_dist_max;
          if (fabsf (dist_dominant) > th_normal_discon_)
          {
          labels[curr_idx].label |= EDGELABEL_HIGH_CURVATURE;
          }
      }
    }
  }
*/


/*

  // METHOD I

  if ((detecting_edge_types_ & EDGELABEL_HIGH_CURVATURE))
  {

    pcl::PointCloud<PointXYZI> nx, ny;
    nx.width = normals_->width;
    nx.height = normals_->height;
    nx.resize (normals_->height*normals_->width);

    ny.width = normals_->width;
    ny.height = normals_->height;
    ny.resize (normals_->height*normals_->width);

    for (uint32_t row=0; row<normals_->height; row++)
    {
      for (uint32_t col=0; col<normals_->width; col++)
      {
        nx (col, row).intensity = normals_->points[row*normals_->width + col].normal_x;
        ny (col, row).intensity = normals_->points[row*normals_->width + col].normal_y;
      }
    }

    pcl::PointCloud<pcl::PointXYZIEdge> img_edge;
    pcl::Edge<PointXYZI, pcl::PointXYZIEdge> edge;
    edge.setHysteresisThresholdLow (th_hc_canny_low_);
    edge.setHysteresisThresholdHigh (th_hc_canny_high_);
    edge.canny (nx, ny, img_edge);

    for (uint32_t row=0; row<labels.height; row++)
    {
      for (uint32_t col=0; col<labels.width; col++)
      {
        if (img_edge (col, row).magnitude == 255.f)
          labels[row * labels.width + col].label |= EDGELABEL_HIGH_CURVATURE;
      }
    }
  }
*/

}

//////////////////////////////////////////////////////////////////////////////
template<typename PointT, typename PointNT, typename PointLT> void
pcl::OrganizedEdgeFromRGBNormals<PointT, PointNT, PointLT>::compute (pcl::PointCloud<PointLT>& labels, std::vector<pcl::PointIndices>& label_indices) const
{
  pcl::Label invalid_pt;
  invalid_pt.label = unsigned (0);
  labels.points.resize (input_->points.size (), invalid_pt);
  labels.width = input_->width;
  labels.height = input_->height;
  
  OrganizedEdgeBase<PointT, PointLT>::extractEdges (labels);
  OrganizedEdgeFromNormals<PointT, PointNT, PointLT>::extractEdges (labels);
  OrganizedEdgeFromRGB<PointT, PointLT>::extractEdges (labels);

  this->assignLabelIndices (labels, label_indices);
}

#define PCL_INSTANTIATE_OrganizedEdgeBase(T,LT)               template class PCL_EXPORTS pcl::OrganizedEdgeBase<T,LT>;
#define PCL_INSTANTIATE_OrganizedEdgeFromRGB(T,LT)            template class PCL_EXPORTS pcl::OrganizedEdgeFromRGB<T,LT>;
#define PCL_INSTANTIATE_OrganizedEdgeFromNormals(T,NT,LT)     template class PCL_EXPORTS pcl::OrganizedEdgeFromNormals<T,NT,LT>;
#define PCL_INSTANTIATE_OrganizedEdgeFromRGBNormals(T,NT,LT)  template class PCL_EXPORTS pcl::OrganizedEdgeFromRGBNormals<T,NT,LT>;

#endif //#ifndef PCL_FEATURES_IMPL_ORGANIZED_EDGE_DETECTION_H_