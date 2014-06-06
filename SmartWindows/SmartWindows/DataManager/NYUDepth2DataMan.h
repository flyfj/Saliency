//////////////////////////////////////////////////////////////////////////
// nyu depth 2 data manager
// jiefeng©2014-6-6
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "DataManager/DataManagerInterface.h"
#include "common.h"


class NYUDepth2DataMan: DataManagerInterface
{
private:

public:
	NYUDepth2DataMan(void)
	{
		imgdir = DB_ROOT + "Datasets\\RGBD_Dataset\\Berkeley\\VOCB3DO\\KinectColor\\";
		depthdir = DB_ROOT + "Datasets\\RGBD_Dataset\\Berkeley\\VOCB3DO\\RegisteredDepthData\\";
		gtdir = DB_ROOT + "Datasets\\RGBD_Dataset\\Berkeley\\VOCB3DO\\Annotations\\";
	}

	bool GetImageList(FileInfos& imgfiles);

	bool GetDepthmapList(FileInfos& depthfiles);

	bool LoadDepthData(const string& depthfile, cv::Mat& depthmap);

	bool LoadGTWins(const FileInfos& imgfiles, map<string, vector<ImgWin>>& gtwins);

};

