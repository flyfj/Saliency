#include "DatasetManager.h"




//////////////////////////////////////////////////////////////////////////

bool VOC07Manager::GetImageList(vector<string>& imgfiles)
{

	return true;
}


ImgWin VOC07Manager::LoadVOC07Box(FileNode& fn)
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


bool VOC07Manager::LoadGTWins(const vector<string>& imgfiles, map<string, vector<ImgWin>>& gtwins)
{
	gtwins.clear();
	string gtdir = "F:\\Datasets\\PASCAL_VOC07\\VOC2007_AnnotationsOpenCV_Readable\\";
	for(size_t i=0; i<imgfiles.size(); i++)
	{
		cv::FileStorage fs;
		string gtfile = gtdir + imgfiles[i] + ".yml";
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
				ImgWin curbox = LoadVOC07Box(*it);
				gtwins[imgfiles[i]].push_back(curbox);
			}
		}
		else
		{
			ImgWin curbox = LoadVOC07Box(fn);
			gtwins[imgfiles[i]].push_back(curbox);
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

DatasetManager::DatasetManager(void)
{
}
