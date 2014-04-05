//////////////////////////////////////////////////////////////////////////
// Define commonly used functions
//////////////////////////////////////////////////////////////////////////

#pragma once


#include "common.h"

#include <io.h>
#include <direct.h>



//using namespace visualsearch;
using namespace cv;

struct FileInfo
{
	string filename;	// no directory name
	string filepath;	// full path
};

struct DirInfo 
{
	string dirname;
	string dirpath;
	float filenum;
};

typedef vector<FileInfo> FileInfos;
typedef vector<DirInfo> DirInfos;

/*	tool functions	*/

class ToolFactory
{

public:

	//////////////////////////////////////////////////////////////////////////
	// conversion functions


	//////////////////////////////////////////////////////////////////////////
	// file io functions
	// type: *.ext
	static void GetFilesFromDir(const string& dir, const string& type, FileInfos& fileInfos);
	static void GetDirsFromDir(const string& dir, DirInfos& dirInfos, const string& outputFile="");
	static void RemoveEmptyDir(const string& dir, const string& type);

	//////////////////////////////////////////////////////////////////////////
	// computation
	static double GetIntegralValue(const cv::Mat& integralImg, cv::Rect box);

	static Rect RefineBox(Rect inBox, Size rangeLimit);
};