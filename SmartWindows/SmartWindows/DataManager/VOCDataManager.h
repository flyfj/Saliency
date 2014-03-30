//////////////////////////////////////////////////////////////////////////
// dataset manager for pascal voc data
// jiefeng@2014-3-30
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "DataManager/DataManagerInterface.h"


// for 07 and 10
class VOCDataManager: public DataManagerInterface
{
private:

	// load ground truth
	ImgWin LoadVOC07Box(FileNode& fn);

public:

	VOCDataManager() 
	{ 
		imgdir = "F:\\Datasets\\PASCAL_VOC07\\VOCtrainval_06-Nov-2007\\VOCdevkit\\VOC2007\\ImageSets\\";
		gtdir = "F:\\Datasets\\PASCAL_VOC07\\VOC2007_AnnotationsOpenCV_Readable\\";
	}

	bool GetImageList(vector<string>& imgfiles);

	bool LoadGTWins(const vector<string>& imgfiles, map<string, vector<ImgWin>>& gtwins);

};

