#include <iostream>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/common/common.h>
#include <pcl/common/transforms.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/filters/voxel_grid.h> 


int 
  main (int argc, char** argv)
{

  pcl::PointCloud<pcl::PointXYZRGB>::Ptr model (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr model_tf1 (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr model_tf2 (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr model_tf3 (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr scene (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr scene_tf1 (new pcl::PointCloud<pcl::PointXYZRGB>);

  // Read PCD files from disk.
  if (pcl::io::loadPCDFile<pcl::PointXYZRGB>(argv[1], *model) != 0)
    {
	return -1;
    }
  if (pcl::io::loadPCDFile<pcl::PointXYZRGB>(argv[2], *scene) != 0)
   {
	return -1;
   }

  // Create the filtering object
  pcl::VoxelGrid<pcl::PointXYZRGB> sor;
  sor.setLeafSize (0.01f, 0.01f, 0.01f);


  sor.setInputCloud (model);
  sor.filter (*model);

  sor.setInputCloud (scene);
  sor.filter (*scene);


  Eigen::Vector3f point1_model(0.0690597,0.121807,0.726103);
  Eigen::Vector3f point2_model(0.0680728,0.0246748,0.637091);
  Eigen::Vector3f point3_model(0.0164271,-0.107744,0.751862);

  Eigen::Vector3f axis_model;
  axis_model = point1_model - point2_model;

  std::cout <<  "axis_model = " << axis_model  << std::endl;

  Eigen::Vector3f axis_model1;
  axis_model1 = point1_model - point3_model;


  Eigen::Vector3f point1_scene(-0.227579,-0.166513,1.08712);
  Eigen::Vector3f point2_scene(-0.213298,-0.180041,0.954568);
  Eigen::Vector3f point3_scene(-0.299508,-0.327723,1.00388);

  Eigen::Vector3f axis_scene;
  axis_scene = point1_scene - point2_scene;

  std::cout <<  "axis_scene = " << axis_scene  << std::endl;

  Eigen::Vector3f axis_scene1;
  axis_scene1 = point1_scene - point3_scene;

/*
  Eigen::Matrix4f First_Transform = Eigen::Matrix4f::Identity();

  First_Transform (0,3) = static_cast<float> (point1_scene(0) -  point1_model(0));
  First_Transform (1,3) = static_cast<float> (point1_scene(1) -  point1_model(1));
  First_Transform (2,3) = static_cast<float> (point1_scene(2) -  point1_model(2));
*/


  Eigen::Matrix4f First_Transform_Model = Eigen::Matrix4f::Identity();

  First_Transform_Model (0,3) = static_cast<float> (-point1_model(0));
  First_Transform_Model (1,3) = static_cast<float> (-point1_model(1));
  First_Transform_Model (2,3) = static_cast<float> (-point1_model(2));



  Eigen::Matrix4f First_Transform_Scene = Eigen::Matrix4f::Identity();

  First_Transform_Scene (0,3) = static_cast<float> (-point1_scene(0));
  First_Transform_Scene (1,3) = static_cast<float> (-point1_scene(1) );
  First_Transform_Scene (2,3) = static_cast<float> (-point1_scene(2));


  axis_model.normalize ();
  axis_scene.normalize ();

  std::cout <<  "axis_scene Normalized = " << axis_scene  << std::endl;


  Eigen::Vector3f Second_Transform_axis = axis_model.cross(axis_scene);

  double rotation = asin (Second_Transform_axis.norm ());

  Second_Transform_axis.normalize ();

  std::cout <<  "Second_Transform_axis = " << Second_Transform_axis  << std::endl;
  std::cout <<  "Rotation Angle = " << rotation << std::endl;

  Eigen::Affine3f Second_Transform (Eigen::AngleAxisf (static_cast<float> (rotation),Second_Transform_axis));

  std::cout <<  "Second_Transform = " << Second_Transform.matrix()  << std::endl;

  std::cout <<  "Total_Transform = " << First_Transform_Model-First_Transform_Scene << std::endl;

/*
  axis_model1.normalize ();
  axis_scene1.normalize ();

  Eigen::Vector3f Third_Transform_axis = axis_model1.cross(axis_scene1);

  double rotation1 = asin (Third_Transform_axis.norm ());

  std::cout <<  "Rotation Angle = " << rotation1  << std::endl;

  Eigen::Affine3f Third_Transform (Eigen::AngleAxisf (static_cast<float> (rotation1),Third_Transform_axis));
*/


  Eigen::Matrix4f First_Transform_Model_2 = Eigen::Matrix4f::Identity();

  First_Transform_Model_2 (0,3) = static_cast<float> (point1_scene(0));
  First_Transform_Model_2 (1,3) = static_cast<float> (point1_scene(1));
  First_Transform_Model_2 (2,3) = static_cast<float> (point1_scene(2));


  pcl::transformPointCloud (*model, *model_tf1, First_Transform_Model);
  pcl::transformPointCloud (*scene, *scene_tf1, First_Transform_Scene);
  pcl::transformPointCloud (*model_tf1, *model_tf2, Second_Transform);
  //pcl::transformPointCloud (*model_tf2, *model_tf3, Third_Transform);

  pcl::transformPointCloud (*model_tf2, *model_tf2, First_Transform_Model_2);



  /// VISUALIZATION Clouds

  boost::shared_ptr<pcl::visualization::PCLVisualizer> model_viewer (new pcl::visualization::PCLVisualizer ("model Cluster viewer"));
  boost::shared_ptr<pcl::visualization::PCLVisualizer> scene_viewer (new pcl::visualization::PCLVisualizer ("scene Cluster viewer"));

  //boost::shared_ptr<pcl::visualization::PCLVisualizer> model_transformed (new pcl::visualization::PCLVisualizer ("model Cluster viewer"));

  model_viewer->setBackgroundColor (255, 255, 255);
  scene_viewer->setBackgroundColor (255, 255, 255);

  pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> model_rgb(model, 0, 0, 255);
  model_viewer->addPointCloud<pcl::PointXYZRGB> (model, model_rgb, "model Cluster viewer");

  pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> scene_rgb(scene, 0, 255, 0);
  scene_viewer->addPointCloud<pcl::PointXYZRGB> (scene, scene_rgb, "scene Cluster viewer");

  //pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> scene_rgb_tf1(scene, 255, 255, 0);
  //scene_viewer->addPointCloud<pcl::PointXYZRGB> (scene_tf1, scene_rgb_tf1, "scene Cluster viewer3");

  //pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> model_tf1_rgb(model_tf1, 255, 0, 0);
  //scene_viewer->addPointCloud<pcl::PointXYZRGB> (model_tf1, model_tf1_rgb,"model Cluster viewer0");

  pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> model_tf2_rgb(model_tf2, 255, 0, 0);
  scene_viewer->addPointCloud<pcl::PointXYZRGB> (model_tf2, model_tf2_rgb,"model Cluster viewer5");

  scene_viewer->addPointCloud<pcl::PointXYZRGB> (model, model_rgb, "model Cluster viewer1");


  model_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "model Cluster viewer");
  scene_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "model Cluster viewer0");
  scene_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "model Cluster viewer1");
  scene_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "model Cluster viewer5");


  scene_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "scene Cluster viewer");

  //scene_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "model Cluster viewer5");
  //scene_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "scene Cluster viewer5");
  //scene_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "cloudf Cluster viewer5");
 

  while (!model_viewer->wasStopped ())
     {
	model_viewer->spinOnce (100);
	boost::this_thread::sleep (boost::posix_time::microseconds (100000));
     }

 return(0);
}