#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
//#include <pcl/features/vfh.h>
#include "vfh.h"
#include "vfh.hpp"
#include <pcl/filters/voxel_grid.h> 
#include <pcl/features/normal_3d.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/histogram_visualizer.h>
#include <string.h>
#include <pcl/common/transforms.h> 
#include <pcl/visualization/cloud_viewer.h>
#include "cvfh.h"
#include "cvfh.hpp"
//#include <pcl/features/cvfh.h>
#include <pcl/point_cloud.h>
#include <pcl/point_representation.h>
#include <pcl/filters/filter.h>


boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer>
            HistoVis (pcl::PointCloud<pcl::VFHSignature308>::ConstPtr cloud1, int length) 
{ 
    boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer> histogram (new pcl::visualization::PCLHistogramVisualizer); 
    histogram->addFeatureHistogram(*cloud1, length, "cloud", 800, 600); 
    histogram->setBackgroundColor(255,255,255); 
    
    return (histogram); 
} 




int
main(int argc, char** argv)
{
	// Objects for storing the point clouds.
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::Normal>::Ptr normals (new pcl::PointCloud<pcl::Normal> ());

        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZ>);

	// Create an empty kdtree object
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ> ());
 
	// Read PCD files from disk.
	if (pcl::io::loadPCDFile<pcl::PointXYZ>(argv[1], *cloud) != 0)
	{
		return -1;
	}

	std::vector<int> NaN_indices;
     	pcl::removeNaNFromPointCloud(*cloud,*cloud, NaN_indices);

	// Create the filtering object
	pcl::VoxelGrid<pcl::PointXYZ> sor;
	sor.setInputCloud (cloud);
	sor.setLeafSize (0.01f, 0.01f, 0.01f);
	sor.filter (*cloud_filtered);

	std::cerr << "PointCloud size= " << cloud->size()  << std::endl;
	std::cerr << "Filtered PointCloud size= " << cloud_filtered->size()  << std::endl;


	// Create the normal estimation class, and pass the input dataset to it
	pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
	ne.setSearchMethod (tree);

	// Use all neighbors in a sphere of radius 3cm
	ne.setRadiusSearch (0.03);

	ne.setInputCloud (cloud_filtered);
	ne.compute (*normals);


	// Compute the features

	// CVFH estimation object.
	pcl::CVFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::VFHSignature308> cvfh;
	cvfh.setInputCloud(cloud_filtered);
	cvfh.setInputNormals(normals);
	cvfh.setSearchMethod(tree);

	// Set the maximum allowable deviation of the normals, for the region segmentation step.
	cvfh.setEPSAngleThreshold(5.0 / 180.0 * M_PI); // 5 degrees.

	// Set the curvature threshold (maximum disparity between curvatures), for the region segmentation step.
	cvfh.setCurvatureThreshold(1.0);

	cvfh.setMinPoints(200);

	// Set to true to normalize the bins of the resulting histogram, 
	// using the total number of points. Note: enabling it will make CVFH
	// invariant to scale just like VFH, but the authors encourage the opposite.
	cvfh.setNormalizeBins(true);

	pcl::PointCloud<pcl::VFHSignature308>::Ptr cvfhs (new pcl::PointCloud<pcl::VFHSignature308>);

	cvfh.compute(*cvfhs);

	std::cerr << "cvfh Descriptor size= " << cvfhs->size()  << std::endl;


        ///////////

	// METHOD #1: Using a Matrix4f

	Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();


	float dep_x=0;
	float dep_y=0;
	float dep_z=0;

	for (size_t i=0;i<cloud_filtered->size();++i){
	dep_x += cloud_filtered->at(i).x;
	dep_y += cloud_filtered->at(i).y;
	dep_z += cloud_filtered->at(i).z; 
	}
	dep_x = dep_x/cloud_filtered->size();
	dep_y = dep_y/cloud_filtered->size();
	dep_z = dep_z/cloud_filtered->size();


	std::cout << "depth= " << dep_x  << ",  " << dep_y  << ",  " << dep_z  << ",  " << std::endl;

	//transform (0,3) = -dep_x;
	//transform (1,3) = -dep_y;
	//transform (2,3) = -dep_z;

	//pcl::transformPointCloud (*cloud_filtered, *cloud_filtered, transform);

        //transform = Eigen::Matrix4f::Identity();


        //pcl::transformPointCloud (*cloud_filtered, *cloud_filtered, transform);

        //transform = Eigen::Matrix4f::Identity();

	// Define a rotation matrix (see https://en.wikipedia.org/wiki/Rotation_matrix)
	float theta; // The angle of rotation in radians
        std::cout << "theta = ";
        std::cin >> theta;
   
	transform (1,1) = cos (theta);
	transform (1,2) = -sin(theta);
	transform (2,1) = sin (theta);
	transform (2,2) = cos (theta);
 
	transform (1,3) = 1.3;

	// Print the transformation
	std::cout << transform << std::endl;


	// Executing the transformation
	pcl::PointCloud<pcl::PointXYZ>::Ptr transformed_cloud (new pcl::PointCloud<pcl::PointXYZ> ());
	pcl::transformPointCloud (*cloud_filtered, *transformed_cloud, transform);


	/////////////

	// Compute Normals
	pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> tne;
	tne.setSearchMethod (tree);
	tne.setRadiusSearch (0.03);
	pcl::PointCloud<pcl::Normal>::Ptr transformed_normals (new pcl::PointCloud<pcl::Normal> ());
	tne.setInputCloud (transformed_cloud);
	tne.compute (*transformed_normals);



	// Compute the features

	// CVFH estimation object.
	pcl::CVFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::VFHSignature308> tcvfh;
	tcvfh.setInputCloud(transformed_cloud);
	tcvfh.setInputNormals(transformed_normals);
	tcvfh.setSearchMethod(tree);

	// Set the maximum allowable deviation of the normals, for the region segmentation step.
	tcvfh.setEPSAngleThreshold(5.0 / 180.0 * M_PI); // 5 degrees.

	// Set the curvature threshold (maximum disparity between curvatures), for the region segmentation step.
	tcvfh.setCurvatureThreshold(1.0);


	// Set to true to normalize the bins of the resulting histogram, 
	// using the total number of points. Note: enabling it will make CVFH
	// invariant to scale just like VFH, but the authors encourage the opposite.
	tcvfh.setNormalizeBins(true);

	pcl::PointCloud<pcl::VFHSignature308>::Ptr tcvfhs (new pcl::PointCloud<pcl::VFHSignature308>);

	tcvfh.compute(*tcvfhs);

	std::cerr << "tcvfh Descriptor size= " << tcvfhs->size()  << std::endl;



        //////////////


	pcl::PointCloud<pcl::VFHSignature308>::Ptr vfhs (new pcl::PointCloud<pcl::VFHSignature308>);

	vfhs->width = 1;
	vfhs->height = 1;
	vfhs->is_dense = false;
	vfhs->points.resize (vfhs->width * vfhs->height);


	std::vector < boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer>, Eigen::aligned_allocator <boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer> > > model_histo_viewer;


	for (size_t i=0; i<cvfhs->size (); ++i)
	{

       		vfhs->at(0)= cvfhs->at(i);

		boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer> model_histo_viewer_init (new pcl::visualization::PCLHistogramVisualizer); 
		model_histo_viewer.push_back(model_histo_viewer_init);
		model_histo_viewer[i]->addFeatureHistogram(*vfhs,308, "model" , 800, 600); 
		model_histo_viewer[i]->setBackgroundColor(255,255,255); 
	}






//	boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer> viewer2; 
//	viewer2 = HistoVis(transformed_vfhs,308); 


	//pcl::visualization::CloudViewer viewer3 ("Cloud");
	//viewer3.showCloud (cloud_filtered);


        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> cloud_h (cloud_filtered, 0, 0, 0);
        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> transformed_cloud_h (transformed_cloud, 0, 0, 0);

        boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer3 (new pcl::visualization::PCLVisualizer ("Transformed Cloud"));

        viewer3->setBackgroundColor (1, 1, 1);
        viewer3->addPointCloud<pcl::PointXYZ> (cloud_filtered, cloud_h, "Transformed Cloud");
        viewer3->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "Transformed Cloud");


	viewer3->addPointCloud<pcl::PointXYZ> (transformed_cloud,transformed_cloud_h, "Transformed Cloud1");
        viewer3->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "Transformed Cloud1");
        



        //////  Draw Normals

        //viewer3->addPointCloudNormals<pcl::PointXYZ, pcl::Normal> (cloud_filtered, normals, 10, 0.05, "normals");

        //viewer3->addPointCloudNormals<pcl::PointXYZ, pcl::Normal> (transformed_cloud, transformed_normals, 10, 0.05, "normals1");


	while (!viewer3->wasStopped ())
	{ 
            viewer3->spinOnce (100);
            boost::this_thread::sleep (boost::posix_time::microseconds (100000));
	}

        
        viewer3->saveScreenshot ("viewer1.png");


  return(0);
}