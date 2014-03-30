#include "DatasetManager.h"



//////////////////////////////////////////////////////////////////////////

DatasetManager::DatasetManager(void)
{
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