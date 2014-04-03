#include "DatasetManager.h"



//////////////////////////////////////////////////////////////////////////

DatasetManager::DatasetManager()
{
}

bool DatasetManager::Init(DatasetName dbname)
{
	dbName = dbname;
	voc_man.Init(dbName);

	return true;
}

void DatasetManager::BrowseDBImages(bool showGT)
{
	FileInfos cimgs, dmaps;
	map<string, vector<ImgWin>> gtwins;
	DataManagerInterface* db_man = NULL;
	if(dbName == DB_BERKELEY3D)
		db_man = &b3d_man;
	if(dbName == DB_VOC07 || dbName == DB_VOC10)
		db_man = &voc_man;

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
		ImgVisualizer::DrawImgWins(color_img, gtwins[cimgs[i].filename]);
			
		if( !dmaps.empty() )
		{
			// show depth image
			Mat dmap;
			b3d_man.LoadDepthData(dmaps[i].filepath, dmap);
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