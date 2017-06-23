
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/filter.h>
#include <pcl/features/normal_3d.h>

#include <pcl/range_image/range_image.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/range_image_visualizer.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/features/range_image_border_extractor.h>
#include <pcl/keypoints/narf_keypoint.h>
#include <pcl/features/narf_descriptor.h>

#include <pcl/features/fpfh.h>
#include <pcl/features/pfh.h>

#include <pcl/registration/correspondence_estimation.h>

#include <pcl/registration/correspondence_rejection_sample_consensus.h>




int main(int argc, char** argv) 
{ 
        time_t start,end, diff; 
        start = time(0); 

        //tgt -> ziel der transformation; src -> die zu transformierende; tmp -> temporäre zur Zwischenspeicherung 
                pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_src (new pcl::PointCloud<pcl::PointXYZ>); 
                pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_tgt (new pcl::PointCloud<pcl::PointXYZ>); 
                pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_tmp (new pcl::PointCloud<pcl::PointXYZ>); 

                
        //Laden der PCD-Files 
                pcl::io::loadPCDFile (argv[1], *cloud_src);	
                pcl::io::loadPCDFile (argv[2], *cloud_tgt);	
                PCL_INFO ("PCD-Files loaded \n"); 

        //Multiple View-Ports 
                boost::shared_ptr<pcl::visualization::PCLVisualizer> MView (new pcl::visualization::PCLVisualizer ("Aligning")); 
                MView->initCameraParameters (); 
                //View-Port1 
                        int v1(0); 
                        MView->createViewPort (0.0, 0.0, 0.5, 1.0, v1); 
                        MView->setBackgroundColor (0, 0, 0, v1); 
                        MView->addText ("Start:View-Port 1", 10, 10, "v1_text", v1); 
                        //PointCloud Farben...verschieben vor v1? 
                        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> green (cloud_src, 0,255,0); 
                        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> red (cloud_tgt, 255,0,0); 

                        MView->addPointCloud (cloud_src, green, "source", v1); 
                        MView->addPointCloud (cloud_tgt, red, "target", v1); 
                        //View-Port2 
                        int v2(0); 
                        MView->createViewPort (0.5, 0.0, 1.0, 1.0, v2); 
                        MView->setBackgroundColor (0, 0, 0, v2); 
                        MView->addText ("Aligned:View-Port 2", 10, 10, "v2_text", v2); 
                        
                        //MView->addPointCloud (cloud_tgt, red, "target2", v2); 
                        //MView->addPointCloud (cloud_src, green, "source2", v2); 
                //Properties for al viewports 
                        MView->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "source"); 
                        MView->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "target"); 
                        //MView->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "target2");	
                        
        //remove NAN-Points 
                std::vector<int> indices1,indices2; 
                pcl::removeNaNFromPointCloud (*cloud_src, *cloud_src, indices1); 
                pcl::removeNaNFromPointCloud (*cloud_tgt, *cloud_tgt, indices2); 
        //Downsampling 
                PCL_INFO ("Downsampling \n"); 
                //temp clouds src & tgt 
                pcl::PointCloud<pcl::PointXYZ>::Ptr ds_src (new pcl::PointCloud<pcl::PointXYZ>); 
                pcl::PointCloud<pcl::PointXYZ>::Ptr ds_tgt (new pcl::PointCloud<pcl::PointXYZ>); 
                pcl::VoxelGrid<pcl::PointXYZ> grid; 
                grid.setLeafSize (0.05, 0.05, 0.05); 
                grid.setInputCloud (cloud_src); 
                grid.filter (*ds_src); 
                grid.setInputCloud (cloud_tgt); 
                grid.filter (*ds_tgt);	
                PCL_INFO ("	Downsampling finished \n"); 

        // Normal-Estimation 
                PCL_INFO ("Normal Estimation \n"); 
                pcl::PointCloud<pcl::Normal>::Ptr norm_src (new pcl::PointCloud<pcl::Normal>); 
                pcl::PointCloud<pcl::Normal>::Ptr norm_tgt (new pcl::PointCloud<pcl::Normal>); 
                        
                pcl::search::KdTree<pcl::PointXYZ>::Ptr tree_src(new pcl::search::KdTree<pcl::PointXYZ>()); 
                pcl::search::KdTree<pcl::PointXYZ>::Ptr tree_tgt(new pcl::search::KdTree<pcl::PointXYZ>()); 
                
                pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne; 
                //Source-Cloud 
                PCL_INFO ("	Normal Estimation - Source \n");	
                ne.setInputCloud (ds_src); 
                ne.setSearchSurface (cloud_src); 
                ne.setSearchMethod (tree_src); 
                ne.setRadiusSearch (0.05); 
                ne.compute (*norm_src); 

                //Target-Cloud 
                PCL_INFO ("	Normal Estimation - Target \n"); 
                ne.setInputCloud (ds_tgt); 
                ne.setSearchSurface (cloud_tgt); 
                ne.setSearchMethod (tree_tgt); 
                ne.setRadiusSearch (0.05); 
                ne.compute (*norm_tgt); 

        // Keypoints _ NARF 
                PCL_INFO ("NARF - Keypoint \n"); 
                //Probleme mit boost daher erstmal ohne 
                pcl::RangeImage range_src; 
                pcl::RangeImage range_tgt; 

                //Header-information for Range Image 
                float angularResolution = (float) (  0.2f * (M_PI/180.0f));  //   0.5 degree in radians 
                float maxAngleWidth     = (float) (360.0f * (M_PI/180.0f));  // 360.0 degree in radians 
                float maxAngleHeight    = (float) (180.0f * (M_PI/180.0f));  // 180.0 degree in radians 
                Eigen::Affine3f sensorPose = (Eigen::Affine3f)Eigen::Translation3f(0.0f, 0.0f, 0.0f); 
                pcl::RangeImage::CoordinateFrame coordinate_frame = pcl::RangeImage::CAMERA_FRAME; 
                float noiseLevel = 0.00; 
                float minRange = 0.0f; 
                int borderSize = 1; 

                //Konvertieren der Point-Cloud in ein Range-Image d.h Cloud mit Range-Value und 3D-Koordinate 
                PCL_INFO ("	NARF - Creating Range-Images\n"); 
                range_src.createFromPointCloud (*cloud_src, angularResolution, maxAngleWidth, maxAngleHeight, sensorPose, coordinate_frame, noiseLevel, minRange, borderSize); 
                range_tgt.createFromPointCloud (*cloud_tgt, angularResolution, maxAngleWidth, maxAngleHeight, sensorPose, coordinate_frame, noiseLevel, minRange, borderSize);	

                //Visualisierung Range-Images 
                pcl::visualization::RangeImageVisualizer range_vis1 ("Range Image"); 
                range_vis1.showRangeImage (range_src); 
                pcl::visualization::RangeImageVisualizer range_vis2 ("Range Image"); 
                range_vis2.showRangeImage (range_tgt); 

                //Extracting NARF-Keypoints 
                pcl::RangeImageBorderExtractor range_image_ba; 
                float support_size = 0.2f; //? 
                        //Keypoints Source 
                pcl::NarfKeypoint narf_keypoint_src (&range_image_ba); 
                narf_keypoint_src.setRangeImage (&range_src); 
                narf_keypoint_src.getParameters ().support_size = support_size; 
                pcl::PointCloud<int> keypoints_ind_src; 
                PCL_INFO ("	NARF - Compute Keypoints - Source\n"); 
                narf_keypoint_src.compute (keypoints_ind_src); 
                PCL_INFO ("	NARF - Found %d Keypoints in Source\n", keypoints_ind_src.size()); 
                        //Keypoints Target 
                pcl::NarfKeypoint narf_keypoint_tgt (&range_image_ba); 
                narf_keypoint_tgt.setRangeImage (&range_tgt); 
                narf_keypoint_tgt.getParameters ().support_size = support_size; 
                pcl::PointCloud<int> keypoints_ind_tgt; 
                PCL_INFO ("	NARF - Compute Keypoints - Target\n"); 
                narf_keypoint_tgt.compute (keypoints_ind_tgt); 
                PCL_INFO ("	NARF - Found %d Keypoints in Target\n", keypoints_ind_tgt.size()); 
        //Umwandlung Keypoints von pcl::PointCloud<int> zu pcl::PointCloud<XYZ>
                pcl::PointCloud<pcl::PointXYZ>::Ptr keypoints_src (new pcl::PointCloud<pcl::PointXYZ>); 
                pcl::PointCloud<pcl::PointXYZ>::Ptr keypoints_tgt (new pcl::PointCloud<pcl::PointXYZ>); 
                
                keypoints_src->width = keypoints_ind_src.points.size(); 
                keypoints_src->height = 1; 
                keypoints_src->is_dense = false; 
                keypoints_src->points.resize (keypoints_src->width * keypoints_src->height);	
                                
                keypoints_tgt->width = keypoints_ind_tgt.points.size(); 
                keypoints_tgt->height = 1; 
                keypoints_tgt->is_dense = false; 
                keypoints_tgt->points.resize (keypoints_tgt->width * keypoints_tgt->height); 
                
                int ind_count=0; 
                
                //source XYZ-CLoud	
                for (size_t i = 0; i < keypoints_ind_src.points.size(); i++) 
                {	
                        ind_count = keypoints_ind_src.points[i]; 
                        //float x = range_src.points[ind_count].x; 
                        
                        keypoints_src->points[i].x = range_src.points[ind_count].x; 
                        keypoints_src->points[i].y = range_src.points[ind_count].y; 
                        keypoints_src->points[i].z = range_src.points[ind_count].z; 
                } 

                //target XYZ-Cloud 
                for (size_t i = 0; i < keypoints_ind_tgt.points.size(); i++) 
                {	
                        ind_count = keypoints_ind_tgt.points[i]; 
                        //float x = range_src.points[ind_count].x; 
                        
                        keypoints_tgt->points[i].x = range_tgt.points[ind_count].x; 
                        keypoints_tgt->points[i].y = range_tgt.points[ind_count].y; 
                        keypoints_tgt->points[i].z = range_tgt.points[ind_count].z; 
                } 

                
        //Feature-Descriptor 
                PCL_INFO ("FPFH - Feature Descriptor\n"); 
                //FPFH	
                        //FPFH Source 
                        //pcl::FPFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::FPFHSignature33> fpfh_est_src; 
                        //pcl::search::KdTree<pcl::PointXYZ>::Ptr tree_fpfh_src (new pcl::search::KdTree<pcl::PointXYZ>); 
                        //fpfh_est_src.setSearchSurface (ds_src);//<-------------- Use All Points for analyzing  the local structure of the cloud 
                        //fpfh_est_src.setInputCloud (keypoints_src); //<------------- But only compute features at the key-points 
                        //fpfh_est_src.setInputNormals (norm_src); 
                        //fpfh_est_src.setRadiusSearch (0.05);	
                        //pcl::PointCloud<pcl::FPFHSignature33>::Ptr fpfh_src (new pcl::PointCloud<pcl::FPFHSignature33>); 
                        //PCL_INFO ("	FPFH - Compute Source\n"); 
                        //fpfh_est_src.compute (*fpfh_src); 
                
                        ////FPFH Target 
                        //pcl::FPFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::FPFHSignature33> fpfh_est_tgt; 
                        //pcl::search::KdTree<pcl::PointXYZ>::Ptr tree_fpfh_tgt (new pcl::search::KdTree<pcl::PointXYZ>); 
                        //fpfh_est_src.setSearchSurface (ds_tgt); 
                        //fpfh_est_tgt.setInputCloud (keypoints_tgt); 
                        //fpfh_est_tgt.setInputNormals (norm_tgt); 
                        //fpfh_est_tgt.setRadiusSearch (0.05); 
                        //pcl::PointCloud<pcl::FPFHSignature33>::Ptr fpfh_tgt (new pcl::PointCloud<pcl::FPFHSignature33>); 
                        //PCL_INFO ("	FPFH - Compute Target\n"); 
                        //fpfh_est_tgt.compute (*fpfh_tgt); 
                        //PCL_INFO ("	FPFH - finished\n"); 
                //PFH-Source 
                PCL_INFO ("PFH - started\n"); 
                pcl::PFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::PFHSignature125> pfh_est_src; 
                pcl::search::KdTree<pcl::PointXYZ>::Ptr tree_pfh_src (new pcl::search::KdTree<pcl::PointXYZ>()); 
                pfh_est_src.setSearchMethod (tree_pfh_src); 
                pfh_est_src.setRadiusSearch (0.1); 
                pfh_est_src.setSearchSurface (ds_src);   
                pfh_est_src.setInputNormals (norm_src); 
                pfh_est_src.setInputCloud (keypoints_src); 
                pcl::PointCloud<pcl::PFHSignature125>::Ptr pfh_src (new pcl::PointCloud<pcl::PFHSignature125>); 
                PCL_INFO ("	PFH - Compute Source\n"); 
                pfh_est_src.compute (*pfh_src); 
                PCL_INFO ("	PFH - finished\n"); 
                //PFH-Target 
                pcl::PFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::PFHSignature125> pfh_est_tgt; 
                pcl::search::KdTree<pcl::PointXYZ>::Ptr tree_pfh_tgt (new pcl::search::KdTree<pcl::PointXYZ>()); 
                pfh_est_tgt.setSearchMethod (tree_pfh_tgt); 
                pfh_est_tgt.setRadiusSearch (0.1); 
                pfh_est_tgt.setSearchSurface (ds_tgt);   
                pfh_est_tgt.setInputNormals (norm_tgt); 
                pfh_est_tgt.setInputCloud (keypoints_tgt); 
                pcl::PointCloud<pcl::PFHSignature125>::Ptr pfh_tgt (new pcl::PointCloud<pcl::PFHSignature125>); 
                PCL_INFO ("	PFH - Compute Target\n"); 
                pfh_est_tgt.compute (*pfh_tgt); 
                PCL_INFO ("	PFH - finished\n"); 

        //Correspondence Estimation 
                PCL_INFO ("Correspondence Estimation\n"); 
                pcl::registration::CorrespondenceEstimation<pcl::PFHSignature125, pcl::PFHSignature125> corEst; 
                corEst.setInputSource (pfh_src); 
                corEst.setInputTarget (pfh_tgt); 
                PCL_INFO (" Correspondence Estimation - Estimate C.\n"); 

                boost::shared_ptr<pcl::Correspondences> cor_all_ptr (new pcl::Correspondences); 
                corEst.determineCorrespondences (*cor_all_ptr);	
                PCL_INFO (" Correspondence Estimation - Found %d Correspondences\n", cor_all_ptr->size());	
                
        //Reject false Correspondences 
                Eigen::Matrix4f transformation; 
                
                PCL_INFO ("Correspondence Rejection Features\n"); 

                //SAC 
                double epsilon_sac = 0.1; // 10cm 
                int iter_sac = 10000; 
                pcl::registration::CorrespondenceRejectorSampleConsensus<pcl::PointXYZ> sac; 

                sac.setInputSource (cloud_src); 
                sac.setInputTarget (cloud_tgt); 
                sac.setInlierThreshold (epsilon_sac); 
                sac.setMaximumIterations (iter_sac); 
                sac.setInputCorrespondences (cor_all_ptr); 

                boost::shared_ptr<pcl::Correspondences> cor_inliers_ptr (new pcl::Correspondences); 
                sac.getCorrespondences (*cor_inliers_ptr); 
                //int sac_size = cor_inliers_ptr->size(); 
                PCL_INFO ("	RANSAC: %d Correspondences Remaining\n", cor_inliers_ptr->size ()); 

                transformation = sac.getBestTransformation(); 
                
                //pcl::registration::trans 
                //pcl::registration::TransformationEstimation<pcl::PointXYZ, pcl::PointXYZ>::Ptr trans_est (new pcl::registration::TransformationEstimation<pcl::PointXYZ, pcl::PointXYZ>); 
                //pcl:: 
                
                //Punktwolke Transformieren 
                pcl::transformPointCloud (*cloud_src, *cloud_tmp, transformation); 
                MView->addPointCloud (cloud_tmp, green, "tmp", v2); 
                MView->addPointCloud (cloud_tgt, red, "target_2", v2); 
                MView->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "tmp"); 
                MView->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "target_2");	
                
                //Zeitdifferenz 
                end = time(0); 
                diff = difftime (end, start); 
                PCL_INFO ("\nDuration: %d", diff); 
                
        // Warten bis Viewer geschlossen wird 
                while (!MView->wasStopped()) 
                        { 
                                MView->spinOnce(100); 
                        } 
                        

}