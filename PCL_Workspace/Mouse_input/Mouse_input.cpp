#include <iostream>
#include <pcl/visualization/pcl_visualizer.h>

void keyboardEventOccurred(const pcl::visualization::KeyboardEvent &event, void* viewer_void)
{
  boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer = *static_cast<boost::shared_ptr<pcl::visualization::PCLVisualizer> *>(viewer_void);
  if (event.getKeySym() == "r" && event.keyDown())
    std::cout << "'r' was pressed" << std::endl;
  if (event.getKeySym() == "h" && event.keyDown())
    std::cout << "'h' was pressed" << std::endl;
}

void mouseEventOccurred(const pcl::visualization::MouseEvent &event, void* viewer_void)
{
  boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer = *static_cast<boost::shared_ptr<pcl::visualization::PCLVisualizer> *>(viewer_void);

/*
  if (event.getButton() == pcl::visualization::MouseEvent::LeftButton &&
      event.getType() == pcl::visualization::MouseEvent::MouseButtonRelease)
    std::cout << "Left mouse button released at position (" << event.getX() << ", " << event.getY() << ")" << std::endl;
*/


    if (event.getButton() == pcl::visualization::MouseEvent::RightButton  &&
      event.getType() == pcl::visualization::MouseEvent::MouseButtonPress)
      std::cout << "Left mouse button pressed at position (" << event.getX() << ", " << event.getY() << ")" << std::endl;
 

  if (event.getButton() == pcl::visualization::MouseEvent::RightButton &&
     event.getType() == pcl::visualization::MouseEvent::MouseButtonRelease)
    std::cout << "Left mouse button released at position (" << event.getX() << ", " << event.getY() << ")" << std::endl;

}

int main()
{
  pcl::visualization::PCLVisualizer::Ptr viewer(new pcl::visualization::PCLVisualizer);
  viewer->addCoordinateSystem();
  viewer->registerKeyboardCallback(keyboardEventOccurred, (void*)&viewer);
  viewer->registerMouseCallback(mouseEventOccurred, (void*)&viewer);
  viewer->spin();
}