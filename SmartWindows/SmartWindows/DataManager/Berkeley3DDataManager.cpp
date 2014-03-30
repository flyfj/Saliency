#include "Berkeley3DDataManager.h"


//////////////////////////////////////////////////////////////////////////

bool Berkeley3DDataManager::GetImageList(FileInfos& imgfiles)
{
	ToolFactory::GetFilesFromDir(imgdir, "*.png", imgfiles);

	return true;
}

bool Berkeley3DDataManager::GetDepthmapList(FileInfos& depthfiles)
{
	ToolFactory::GetFilesFromDir(depthdir, "*_smooth.png", depthfiles);

	return true;
}

bool Berkeley3DDataManager::LoadDepthData(const string& depthfile, cv::Mat& depthmap)
{
	/*cv::FileStorage fs;
	if( !fs.open(depthfile, cv::FileStorage::READ) )
	{
	cerr<<"Can't open depth file: "<<depthfile<<endl;
	return false;
	}

	fs["depth"] >> depthmap;
	fs.release();*/

	depthmap = cv::imread(depthfile, 0);
	depthmap.convertTo(depthmap, CV_32F, 256);

	return true;
}