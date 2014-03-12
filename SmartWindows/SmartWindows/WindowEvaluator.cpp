#include "WindowEvaluator.h"


WindowEvaluator::WindowEvaluator(void)
{
	gtdir = "F:\\Datasets\\PASCAL_VOC07\\VOCtrainval_06-Nov-2007\\VOCdevkit\\VOC2007\\ImageSets\\";
}

bool WindowEvaluator::LoadVOC07Boxes(const std::string& filename, std::vector<ImgWin>& boxes)
{
	boxes.clear();
	cv::FileStorage fs;
	if( !fs.open(filename, cv::FileStorage::READ) )
	{
		cerr<<"Can't open gt file: "<<filename<<endl;
		return false;
	}
	cv::FileNode fn = fs["annotation"]["object"];
	if (fn.isSeq())
	{
		for (cv::FileNodeIterator it = fn.begin(), it_end = fn.end(); it != it_end; it++)
		{
			ImgWin curbox;
			std::string strXmin, strYmin, strXmax, strYmax;
			(*it)["bndbox"]["xmin"] >> strXmin;
			(*it)["bndbox"]["ymin"] >> strYmin;
			(*it)["bndbox"]["xmax"] >> strXmax;
			(*it)["bndbox"]["ymax"] >> strYmax;
			curbox.box.x = atoi(strXmin.c_str());
			curbox.box.y = atoi(strYmin.c_str());
			curbox.box.width = atoi(strXmax.c_str()) - curbox.box.x;
			curbox.box.height = atoi(strYmax.c_str()) - curbox.box.y;
			boxes.push_back(curbox);
		}
	}
	else
	{
		ImgWin curbox;
		std::string strXmin, strYmin, strXmax, strYmax;
		fn["bndbox"]["xmin"] >> strXmin;
		fn["bndbox"]["ymin"] >> strYmin;
		fn["bndbox"]["xmax"] >> strXmax;
		fn["bndbox"]["ymax"] >> strYmax;
		curbox.box.x = atoi(strXmin.c_str());
		curbox.box.y = atoi(strYmin.c_str());
		curbox.box.width = atoi(strXmax.c_str()) - curbox.box.x;
		curbox.box.height = atoi(strYmax.c_str()) - curbox.box.y;
		boxes.push_back(curbox);
	}

	return true;
}
