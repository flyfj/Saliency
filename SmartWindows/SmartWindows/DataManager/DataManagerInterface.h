

#pragma once

#include "common.h"
#include "Tools.h"

enum DatasetName
{
	DB_VOC07,
	DB_VOC10,
	DB_MSRA,
	DB_BERKELEY3D,
	DB_NYU_RGBD
};


// an abstract interface for dataset management
// 1. get image files
// 2. parse ground truth annotation
class DataManagerInterface
{
public:

	// data
	string imgdir;
	string depthdir;
	string gtdir;

	// methods
	// get all image files
	virtual bool GetImageList(FileInfos& imgfiles) { return true; };
	// get all depth maps
	virtual bool GetDepthmapList(FileInfos& depthfiles) { return true; };
	// load depth map data
	virtual bool LoadDepthData(const string& depthfile, cv::Mat& depthmap) { return true; };
	// get corresponding labels for image windows
	virtual bool LoadGTWins(const FileInfos& imgfiles, map<string, vector<ImgWin>>& gtwins) { return true; };
};

