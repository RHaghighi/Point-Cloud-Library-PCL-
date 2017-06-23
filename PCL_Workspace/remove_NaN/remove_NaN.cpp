#include <pcl/io/pcd_io.h>
#include <pcl/filters/filter.h>
 
#include <iostream>
#include <string>
 
int
main(int argc, char** argv)
{

        for (size_t i=1; i<argc; ++i)
            {
        
	        // Object for storing the point cloud.
	        pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
 
        	// Read a PCD file from disk.
        	if (pcl::io::loadPCDFile<pcl::PointXYZRGBA>(argv[i], *cloud) != 0)
        	   {
         	       return -1;
              	   }
 
	        // The mapping tells you to what points of the old cloud the new ones correspond,
	        // but we will not use it.
	        std::vector<int> mapping;
	        pcl::removeNaNFromPointCloud(*cloud, *cloud, mapping);
 
                std::string filename;
                filename.append(argv[i]);
                filename.replace(filename.end()-4,filename.end(),"_new.pcd");
	        // Save it back.
	        pcl::io::savePCDFileASCII(filename, *cloud);
  
                filename.erase(filename.begin(),filename.end());

             }
}