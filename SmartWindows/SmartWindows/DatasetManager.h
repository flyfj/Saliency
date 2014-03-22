//////////////////////////////////////////////////////////////////////////
// manage datasets
// jiefeng@2014-3-22
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "common.h"

enum DatasetName
{
	DB_VOC07,
	DB_VOC10,
	DB_MSRA,
	DB_NYU_RGBD
};

// general interface for accessing dataset images
class DatasetInterface
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


class VOC07Manager: public DatasetInterface
{
private:

	// load ground truth
	ImgWin LoadVOC07Box(FileNode& fn);

public:

	VOC07Manager();

	bool GetImageList(vector<string>& imgfiles);

	bool LoadGTWins(const vector<string>& imgfiles, map<string, vector<ImgWin>>& gtwins);
};


class DatasetManager
{
private:

	VOC07Manager voc07_man;

public:
	DatasetManager(void);



	//////////////////////////////////////////////////////////////////////////
	// database analysis
	//////////////////////////////////////////////////////////////////////////
	// generate positive and negative object windows
	bool GenerateWinSamps(const vector<string>& imgnames);

	
};

