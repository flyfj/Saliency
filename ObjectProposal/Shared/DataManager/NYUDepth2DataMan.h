//////////////////////////////////////////////////////////////////////////
// nyu depth 2 data manager
// jiefeng©2014-6-6
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "DataManagerInterface.h"
#include "../Common/common_libs.h"
#include "../Processors/ShapeAnalyzer.h"

using namespace visualsearch;

class NYUDepth2DataMan: DataManagerInterface
{
private:
	string DB_ROOT;

public:
	NYUDepth2DataMan(void)
	{
		DB_ROOT = "E:\\";
		imgdir = DB_ROOT + "Datasets\\RGBD_Dataset\\NYU\\Depth2\\";
		depthdir = DB_ROOT + "Datasets\\RGBD_Dataset\\NYU\\Depth2\\";
		gtdir = DB_ROOT + "Datasets\\RGBD_Dataset\\NYU\\Depth2\\";
	}

	bool GetImageList(FileInfos& imgfiles);

	bool GetDepthmapList(FileInfos& depthfiles);

	bool LoadDepthData(const string& depthfile, cv::Mat& depthmap);

	bool LoadGTMasks(const FileInfos& imgfiles, map<string, vector<Mat>>& gtmasks);

	bool LoadGTWins(const FileInfos& imgfiles, map<string, vector<ImgWin>>& gtwins);

};

