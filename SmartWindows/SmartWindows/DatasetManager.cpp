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
	curbox.x = atoi(strXmin.c_str());
	curbox.y = atoi(strYmin.c_str());
	curbox.width = atoi(strXmax.c_str()) - curbox.x;
	curbox.height = atoi(strYmax.c_str()) - curbox.y;

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


bool DatasetManager::GenerateWinSamps(const vector<string>& imgnames)
{
	// load boxes
	map<string, vector<ImgWin>> gtwins;
	//LoadVOC07Boxes(imgnames, gtwins);

	char str[30];
	string imgdir = "F:\\Datasets\\PASCAL_VOC07\\VOCtrainval_06-Nov-2007\\VOCdevkit\\VOC2007\\ImageSets\\";
	string possave = "F:\\Datasets\\PASCAL_VOC07\\pos\\";
	string negsave = "F:\\Datasets\\PASCAL_VOC07\\neg\\";
	for (size_t i=0; i<imgnames.size(); i++)
	{
		Mat curimg = imread(imgdir + imgnames[i] + ".jpg");
		imshow("curimg", curimg);
		for(size_t j=0; j<gtwins[imgnames[i]].size(); j++)
		{
			Mat objimg = curimg(gtwins[imgnames[i]][j]);
			imshow("obj", objimg);
			sprintf(str, "%d", j);
			string savefile = possave + imgnames[i] + string(str) + ".jpg";
			imwrite(savefile, objimg);
		}

		// randomly generate negative windows
		int negnum = 5;
		for(int j=0; j<negnum; j++)
		{
			Rect negbox;
			negbox.x = rand() % curimg.cols;
			negbox.y = rand() % curimg.rows;
		}

	}

	return true;
}