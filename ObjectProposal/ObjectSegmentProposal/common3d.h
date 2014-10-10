
#pragma once

// standard library
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <thread>
#include <direct.h>
#include <time.h>
using namespace std;

// boost
//#include <boost/shared_ptr.hpp>

// vtk
#include <vtkSmartPointer.h>

// pcl headers
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
//#include <pcl/correspondence.h>

#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/io/obj_io.h>

#include <pcl/common/transforms.h>
#include <pcl/console/parse.h>

#include <pcl/keypoints/uniform_sampling.h>
#include <pcl/keypoints/iss_3d.h>
//
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>

#include <pcl/features/normal_3d_omp.h>
#include <pcl/features/fpfh.h>
#include <pcl/features/shot_omp.h>
#include <pcl/features/board.h>
#include <pcl/features/esf.h>
//
//#include <pcl/registration/icp.h>
//
#include <pcl/kdtree/kdtree_flann.h>

//#include <pcl/recognition/cg/hough_3d.h>
//#include <pcl/recognition/cg/geometric_consistency.h>

#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/cloud_viewer.h>

//#include <pcl/ml/decision_forest.h>

// type definitions
typedef pcl::PointXYZ PointType;
typedef pcl::Normal NormalType;
typedef pcl::SHOT352 PointDescriptorType;
typedef pcl::PointCloud<PointType> PointCloudPoints;
typedef pcl::PointCloud<NormalType> PointCloudNormals;
typedef pcl::PointCloud<PointDescriptorType> PointCloudDescriptors;