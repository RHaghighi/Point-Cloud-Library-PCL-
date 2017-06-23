#include <pcl/io/pcd_io.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/pfh.h>
#include <pcl/filters/voxel_grid.h> 
#include <pcl/visualization/pcl_visualizer.h>
int
main(int argc, char** argv)
{
	// Object for storing the point cloud.
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	// Object for storing the normals.
	pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
	// Object for storing the PFH descriptors for each point.
	pcl::PointCloud<pcl::PFHSignature125>::Ptr descriptors(new pcl::PointCloud<pcl::PFHSignature125>());
 
	// Read a PCD file from disk.
	if (pcl::io::loadPCDFile<pcl::PointXYZ>(argv[1], *cloud) != 0)
	{
		return -1;
	}
 
	// Note: you would usually perform downsampling now. It has been omitted here
	// for simplicity, but be aware that computation can take a long 

        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZ>);
  
        pcl::VoxelGrid<pcl::PointXYZ> sor;
        sor.setInputCloud (cloud);
        sor.setLeafSize (0.01f, 0.01f, 0.01f);
        sor.filter (*cloud_filtered);
	// Estimate the normals.
	pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> normalEstimation;
	normalEstimation.setInputCloud(cloud_filtered);
	normalEstimation.setRadiusSearch(0.03);
	pcl::search::KdTree<pcl::PointXYZ>::Ptr kdtree(new pcl::search::KdTree<pcl::PointXYZ>);
	normalEstimation.setSearchMethod(kdtree);
	normalEstimation.compute(*normals);
 
	// PFH estimation object.
	pcl::PFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::PFHSignature125> pfh;
	pfh.setInputCloud(cloud_filtered);
	pfh.setInputNormals(normals);
	pfh.setSearchMethod(kdtree);
	// Search radius, to look for neighbors. Note: the value given here has to be
	// larger than the radius used to estimate the normals.
	pfh.setRadiusSearch(0.05);
 
	pfh.compute(*descriptors);


return(0);
        
}