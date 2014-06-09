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

	static float ComputeWinMatchScore(const Rect& qwin, const Rect& gwin)
	{
		Rect box1(qwin.x, qwin.y, qwin.width, qwin.height);
		Rect box2(gwin.x, gwin.y, gwin.width, gwin.height);

		Rect interBox = box1 & box2;
		Rect unionBox = box1 | box2;

		if(unionBox.area() > 0)
			return (float)interBox.area() / unionBox.area();
		else
			return 0;
	}

	static float compute_downsample_ratio(cv::Size oldSz, float downSampleFactor, cv::Size& newSz);

	static ImgWin GetContextWin(int imgw, int imgh, ImgWin win, float ratio);

	static bool DrawHist(cv::Mat& canvas, cv::Size canvas_size, int max_val, const cv::Mat& hist);
};