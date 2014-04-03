//////////////////////////////////////////////////////////////////////////
// manage datasets
// jiefeng@2014-3-22
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "ImgVisualizer.h"
#include "DataManager/VOCDataManager.h"
#include "DataManager/Berkeley3DDataManager.h"


class DatasetManager
{
private:

	VOCDataManager voc_man;
	Berkeley3DDataManager b3d_man;

	DatasetName dbName;

public:

	DatasetManager();

	bool Init(DatasetName dbname);

	//////////////////////////////////////////////////////////////////////////
	// database analysis
	//////////////////////////////////////////////////////////////////////////
	// loop over all images for visualization
	void BrowseDBImages(bool showGT = true);
	// generate positive and negative object windows
	bool GenerateWinSamps(DatasetName dname);

};

