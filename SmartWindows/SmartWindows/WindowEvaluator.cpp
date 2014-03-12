#include "WindowEvaluator.h"


WindowEvaluator::WindowEvaluator(void)
{
	gtdir = "F:\\Datasets\\PASCAL_VOC07\\VOCtrainval_06-Nov-2007\\VOCdevkit\\VOC2007\\ImageSets\\";
}

//////////////////////////////////////////////////////////////////////////

ImgWin WindowEvaluator::LoadBox(FileNode& fn)
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

	return curbox;
}

bool WindowEvaluator::LoadVOC07Boxes(const vector<std::string>& imgnames, std::map<string, vector<ImgWin>>& gtwins)
{
	gtwins.clear();

	for(size_t i=0; i<imgnames.size(); i++)
	{
		cv::FileStorage fs;
		string gtfile = gtdir + imgnames[i] + ".yml";
		if( !fs.open(gtfile, cv::FileStorage::READ) )
		{
			cerr<<"Can't open gt file: "<<gtfile<<endl;
			return false;
		}

		cv::FileNode fn = fs["annotation"]["object"];
		if (fn.isSeq())
		{
			for (cv::FileNodeIterator it = fn.begin(), it_end = fn.end(); it != it_end; it++)
			{
				ImgWin curbox = LoadBox(*it);
				gtwins[imgnames[i]].push_back(curbox);
			}
		}
		else
		{
			ImgWin curbox = LoadBox(fn);
			gtwins[imgnames[i]].push_back(curbox);
		}
	}

	return true;
}
