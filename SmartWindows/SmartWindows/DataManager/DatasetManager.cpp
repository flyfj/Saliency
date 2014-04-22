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
		ImgVisualizer::DrawImgWins("color", color_img, gtwins[cimgs[i].filename]);
		moveWindow("color", 100, 300);	

		if( !dmaps.empty() )
		{
			// show depth image
			Mat dmap;
			db_man->LoadDepthData(dmaps[i].filepath, dmap);
			ImgVisualizer::DrawFloatImg(cimgs[i].filename, dmap, Mat());
			moveWindow(cimgs[i].filename, color_img.cols + 150, 300);
		}
			
		if( waitKey(0) == 'q')
			break;

		destroyAllWindows();
	}

	destroyAllWindows();
	// reset
	db_man = NULL;
}


bool DatasetManager::GenerateWinSamps()
{
	char str[100];
	
	string possave = DB_ROOT + "Datasets\\objectness\\pos\\";
	string negsave = DB_ROOT + "Datasets\\objectness\\neg\\";
	_mkdir(possave.c_str());
	_mkdir(negsave.c_str());

	FileInfos cimgs, dmaps;
	map<string, vector<ImgWin>> gtwins;

	db_man->GetImageList(cimgs);
	db_man->GetDepthmapList(dmaps);
	bool useDepth = !dmaps.empty();
	//assert(cimgs.size() == dmaps.size());

	db_man->LoadGTWins(cimgs, gtwins);

	for (size_t i=0; i<cimgs.size(); i++)
	{
		cout<<"Image: "<<i<<endl;
		// show color image
		cv::Mat color_img = cv::imread(cimgs[i].filepath);
		ImgVisualizer::DrawImgWins("color", color_img, gtwins[cimgs[i].filename]);
		moveWindow("color", 100, 300);
		
		// show depth image
		Mat dmap;
		if( useDepth )
		{
			db_man->LoadDepthData(dmaps[i].filepath, dmap);
			ImgVisualizer::DrawFloatImg(cimgs[i].filename, dmap, Mat());
			moveWindow(cimgs[i].filename, color_img.cols + 150, 300);
		}

		// get positive windows
		for(size_t j=0; j<gtwins[cimgs[i].filename].size(); j++)
		{
			Mat objimg = color_img(gtwins[cimgs[i].filename][j]);
			imshow("obj", objimg);
			sprintf(str, "%d", j);
			string savefile = possave + cimgs[i].filename + string(str) + ".jpg";
			imwrite(savefile, objimg);

			if(useDepth)
			{
				Mat dobjimg = dmap(gtwins[cimgs[i].filename][j]);
				ImgVisualizer::DrawFloatImg("dobj", dobjimg, Mat());
				savefile = possave + cimgs[i].filename + string(str) + "_d.xml";
				FileStorage fs(savefile, FileStorage::WRITE);
				fs<<"dmap"<<dobjimg;
				fs.release();
			}

			//waitKey(0);
			//destroyAllWindows();
		}

		// get negative windows

		if( waitKey(0) == 'q')
			break;

		destroyAllWindows();
	}

	

		//// randomly generate negative windows
		//int negnum = 5;
		//for(int j=0; j<negnum; j++)
		//{
		//	Rect negbox;
		//	negbox.x = rand() % curimg.cols;
		//	negbox.y = rand() % curimg.rows;
		//}


	return true;
}