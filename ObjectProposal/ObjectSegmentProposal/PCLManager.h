//////////////////////////////////////////////////////////////////////////

#pragma once

#include "common3d.h"
#include "Common\Tools\ImgVisualizer.h"

enum PCLOptions
{
	PCL_NORMAL_INTEGRAL,
	PCL_NORMAL_REGULAR,
	PCL_KEYPOINT_UNIFORM,
	PCL_KEYPOINT_DETECT,
	PCL_KEYPOINT_VOXELGRID,
	PCL_FEATURE_SHOT,
	PCL_FEATURE_FPFH,
	PCL_FEATURE_ESF
};

enum FeatureType3D
{
	FEAT_3D_SHOT,
	FEAT_3D_ESF,
	FEAT_3D_FPFH
};

// implement general functions for pcl
class PCLManager
{
public:

	float model_ss;		// sample size
	float nn_k;			// normal k
	float nn_rad;		// normal radius
	float rf_rad;		// reference frame radius
	float desc_rad;		// descriptor size
	float desc_k;

public:
	PCLManager(void);

	// io
	bool WritePCD();
	
	// filter noise and normalize scale
	bool PreprocessPCL(PointCloudPoints::Ptr& input);

	void NormalizePCLScale(const PointCloudPoints::Ptr cloud);

	// features
	bool Detect3DKeypoints(const PointCloudPoints::ConstPtr input, const PCLOptions option, PointCloudPoints::Ptr output);

	bool EstimateNormals(const PointCloudPoints::Ptr input, PCLOptions option, PointCloudNormals::Ptr normals);

	bool ComputePointDescriptors(const PointCloudPoints::Ptr input, const PointCloudPoints::Ptr keypoints, 
		const FeatureType3D option, PointCloudDescriptors::Ptr feats);

	bool GetPCLRange(const PointCloudPoints::Ptr input, pcl::PointXYZ& minp, pcl::PointXYZ& maxp);

	bool SerializeDescriptors(const string& fn, const PointCloudDescriptors::Ptr descs, const FeatureType3D desc_type);
	bool DeserializeDescriptors(const string& fn, PointCloudDescriptors::Ptr descs, const FeatureType3D desc_type);

	// visualization
	bool VisualizePCL(const PointCloudPoints::Ptr input, const PointCloudNormals::Ptr normals);
	bool ShowKeypoints(const PointCloudPoints::Ptr input, const PointCloudPoints::Ptr keypoints);

	
};

