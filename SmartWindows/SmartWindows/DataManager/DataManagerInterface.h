

#pragma once

#include "common.h"

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
	string gtdir;

	// methods
	// get all image files
	virtual bool GetImageList(vector<string>& imgfiles) = 0;
	// get corresponding labels for image windows
	virtual bool LoadGTWins(const vector<string>& imgfiles, map<string, vector<ImgWin>>& gtwins) = 0;
};

