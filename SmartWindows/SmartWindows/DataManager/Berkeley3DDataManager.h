//////////////////////////////////////////////////////////////////////////
// for berkeley kinect 3d dataset
// jiefeng@2014-3-30
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "DataManager/DataManagerInterface.h"

class Berkeley3DDataManager: public DataManagerInterface
{
private:



public:
	Berkeley3DDataManager(void)
	{
		imgdir = "E:\\Datasets\\RGBD Dataset\\Berkeley\\VOCB3DO\\KinectColor\\";
		gtdir = "E:\\Datasets\\RGBD Dataset\\Berkeley\\VOCB3DO\\DepthData\\";
	}

	bool GetImageList(vector<string>& imgfiles);

	bool LoadGTWins(const vector<string>& imgfiles, map<string, vector<ImgWin>>& gtwins);
};

