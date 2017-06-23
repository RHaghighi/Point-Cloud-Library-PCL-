#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include <pcl/filters/voxel_grid.h> 
#include <pcl/features/normal_3d.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/histogram_visualizer.h>
#include <string.h>
#include <pcl/common/transforms.h> 
#include <pcl/visualization/cloud_viewer.h>

#include <pcl/point_cloud.h>
#include <pcl/point_representation.h>

#include <pcl/features/gfpfh.h>
#include <pcl/features/impl/gfpfh.hpp>

boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer>
            HistoVis (pcl::PointCloud<pcl::GFPFHSignature16>::ConstPtr cloud1, int length) 
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

	// Create the filtering object
	pcl::VoxelGrid<pcl::PointXYZ> sor;
	sor.setInputCloud (cloud);
	sor.setLeafSize (0.03f, 0.03f, 0.03f);
	sor.filter (*cloud_filtered);

	std::cerr << "PointCloud size= " << cloud->size()  << std::endl;
	std::cerr << "Filtered PointCloud size= " << cloud_filtered->size()  << std::endl;


	// Create the normal estimation class, and pass the input dataset to it
	pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
	ne.setSearchMethod (tree);

	// Use all neighbors in a sphere of radius 3cm
	ne.setRadiusSearch (0.09);

	//std::cerr << "Normal size= " << normals->size()  << std::endl;


	  // Create the gfpfh estimation class, and pass the input dataset+normals to it
	pcl::GFPFHEstimation<pcl::PointXYZ, pcl::PointXYZL, pcl::GFPFHSignature16> gfpfh;
	gfpfh.setSearchMethod (tree);

	// Output datasets
	pcl::PointCloud<pcl::GFPFHSignature16>::Ptr gfpfhs (new pcl::PointCloud<pcl::GFPFHSignature16> ());



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

	//std::cout << "depth= " << dep << std::endl;

	transform (0,3) = -dep_x;
	transform (1,3) = -dep_y;
	transform (2,3) = -dep_z;

	pcl::transformPointCloud (*cloud_filtered, *cloud_filtered, transform);

        transform = Eigen::Matrix4f::Identity();

	// Define a rotation matrix (see https://en.wikipedia.org/wiki/Rotation_matrix)
	float theta; // The angle of rotation in radians
        std::cout << "theta = ";
        std::cin >> theta;
   
	transform (0,0) = cos (theta);
	transform (2,0) = -sin(theta);
	transform (0,2) = sin (theta);
	transform (2,2) = cos (theta);
	//    (row, column)

	// Define a translation of 2.5 meters on the x axis.
	transform (0,3) = 0;

	// Print the transformation
	printf ("Method #1: using a Matrix4f\n");
	std::cout << transform << std::endl;

	// Executing the transformation
	pcl::PointCloud<pcl::PointXYZ>::Ptr transformed_cloud (new pcl::PointCloud<pcl::PointXYZ> ());
	// You can either apply transform_1 or transform_2; they are the same
	pcl::transformPointCloud (*cloud_filtered, *transformed_cloud, transform);


        //////////////

        pcl::PointCloud<pcl::PointXYZL>::Ptr labeled (new pcl::PointCloud<pcl::PointXYZL>);
        pcl::copyPointCloud (*cloud_filtered, *labeled);


	// Compute the features

	ne.setInputCloud (cloud_filtered);
	ne.compute (*normals);

	gfpfh.setInputCloud (cloud_filtered);
	gfpfh.setInputLabels (labeled);

	// Compute the features
	gfpfh.compute (*gfpfhs);

        std::cerr << "Descriptor size= " << gfpfhs->size()  << std::endl;
        std::cerr << "Descriptor " << gfpfhs->at(0)  << std::endl;


        pcl::PointCloud<pcl::PointXYZL>::Ptr transformed_labeled (new pcl::PointCloud<pcl::PointXYZL>);
        pcl::copyPointCloud (*transformed_cloud, *transformed_labeled);

	// Compute Normals
	pcl::PointCloud<pcl::Normal>::Ptr transformed_normals (new pcl::PointCloud<pcl::Normal> ());
	ne.setInputCloud (transformed_cloud);
	ne.compute (*transformed_normals);


	// Compute gfpfh 
	gfpfh.setInputCloud (transformed_cloud);
	gfpfh.setInputLabels (transformed_labeled);
	pcl::PointCloud<pcl::GFPFHSignature16>::Ptr transformed_gfpfhs (new pcl::PointCloud<pcl::GFPFHSignature16> ());

	gfpfh.compute (*transformed_gfpfhs);

	



	boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer> viewer; 
	viewer = HistoVis(gfpfhs,16); 

	boost::shared_ptr<pcl::visualization::PCLHistogramVisualizer> viewer2; 
	viewer2 = HistoVis(transformed_gfpfhs,16); 

	//pcl::visualization::CloudViewer viewer3 ("Cloud");
	//viewer3.showCloud (cloud_filtered);


        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> cloud_h (cloud_filtered, 0, 255, 0);
        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> transformed_cloud_h (transformed_cloud, 255, 0, 0);

        boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer3 (new pcl::visualization::PCLVisualizer ("Transformed Cloud"));
        viewer3->addPointCloud<pcl::PointXYZ> (cloud_filtered, cloud_h, "Transformed Cloud");

        viewer3->addPointCloud<pcl::PointXYZ> (transformed_cloud,transformed_cloud_h);

	while (!viewer3->wasStopped ())
	{ 
            viewer3->spinOnce (100);
            boost::this_thread::sleep (boost::posix_time::microseconds (100000));
	}


  return(0);
}