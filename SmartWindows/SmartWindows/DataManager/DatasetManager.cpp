#include "DatasetManager.h"



//////////////////////////////////////////////////////////////////////////


bool DatasetManager::Init(DatasetName dbname)
{
	dbName = dbname;
	if(db_man != NULL)
	{
		delete db_man;
		db_man = NULL;
	}

	if (dbName == DB_BERKELEY3D)
		db_man = new Berkeley3DDataManager();
	if(dbName == DB_VOC07 || dbName == DB_VOC10)
	{
		db_man = new VOCDataManager();
		((VOCDataManager*)db_man)->Init(dbName);
	}

	return true;
}

bool DatasetManager::GetImageList(FileInfos& imgfiles)
{
	if(db_man == NULL) return false;

	return db_man->GetImageList(imgfiles);
}


bool DatasetManager::LoadGTWins(const FileInfos& imgfiles, map<string, vector<ImgWin>>& gtwins)
{
	if(db_man == NULL) return false;

	return db_man->LoadGTWins(imgfiles, gtwins);
}


void DatasetManager::BrowseDBImages(bool showGT)
{
	FileInfos cimgs, dmaps;
	map<string, vector<ImgWin>> gtwins;

	db_man->GetImageList(cimgs);
	db_man->GetDepthmapList(dmaps);
	//assert(cimgs.size() == dmaps.size());

	if(showGT)
		db_man->LoadGTWins(cimgs, gtwins);

	for (size_t i=0; i<cimgs.size(); i++)
	{
		cout<<"Image: "<<i<<endl;
		// show color image
		cv::Mat color_img = cv::imread(cimgs[i].filepath);
		ImgVisualizer::DrawImgWins("", color_img, gtwins[cimgs[i].filename]);
			
		if( !dmaps.empty() )
		{
			// show depth image
			Mat dmap;
			db_man->LoadDepthData(dmaps[i].filepath, dmap);
			ImgVisualizer::DrawFloatImg(cimgs[i].filename, dmap, Mat());
		}
			
		if( waitKey(0) == 'q')
			break;

		destroyAllWindows();
	}

	destroyAllWindows();
	// reset
	db_man = NULL;
}


//bool DatasetManager::GenerateWinSamps(DatasetName dname)
//{
//	// load boxes
//	map<string, vector<ImgWin>> gtwins;
//	
//	vector<string> imgnames;
//	if(dname == DB_VOC07)
//		voc07_man.GetImageList(imgnames);
//
//
//	char str[30];
//	
//	string possave = "F:\\Datasets\\PASCAL_VOC07\\pos\\";
//	string negsave = "F:\\Datasets\\PASCAL_VOC07\\neg\\";
//	for (size_t i=0; i<imgnames.size(); i++)
//	{
//		Mat curimg = imread(imgdir + imgnames[i] + ".jpg");
//		imshow("curimg", curimg);
//		for(size_t j=0; j<gtwins[imgnames[i]].size(); j++)
//		{
//			Mat objimg = curimg(gtwins[imgnames[i]][j]);
//			imshow("obj", objimg);
//			sprintf(str, "%d", j);
//			string savefile = possave + imgnames[i] + string(str) + ".jpg";
//			imwrite(savefile, objimg);
//		}
//
//		// randomly generate negative windows
//		int negnum = 5;
//		for(int j=0; j<negnum; j++)
//		{
//			Rect negbox;
//			negbox.x = rand() % curimg.cols;
//			negbox.y = rand() % curimg.rows;
//		}
//
//	}
//
//	return true;
//}