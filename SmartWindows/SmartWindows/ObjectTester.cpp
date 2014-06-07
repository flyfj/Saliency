#include "ObjectTester.h"


ObjectTester::ObjectTester(void)
{
}

//////////////////////////////////////////////////////////////////////////

void ObjectTester::TestObjectRanking(const DatasetName& dbname)
{
	// load dataset
	FileInfos img_fns;
	FileInfos dmap_fns;
	map<string, vector<ImgWin>> rawgtwins;

	if(dbname == DB_NYU2_RGBD)
	{
		NYUDepth2DataMan nyudata;
		nyudata.GetImageList(img_fns);
		nyudata.GetDepthmapList(dmap_fns);
		nyudata.LoadGTWins(img_fns, rawgtwins);
	}
	if(dbname == DB_BERKELEY3D)
	{
		Berkeley3DDataManager berkeleydata;
		berkeleydata.GetImageList(img_fns);
		berkeleydata.GetDepthmapList(dmap_fns);
		berkeleydata.LoadGTWins(img_fns, rawgtwins);
	}

	GenericObjectDetector detector;
	if( !detector.InitBingObjectness() )
		return;

	SalientRegionDetector saldet;
	vector<vector<ImgWin>> objdetwins, saldetwins, gtwins;
	for (int i=0; i<img_fns.size(); i++)
	{
		Mat curimg = imread(img_fns[i].filepath);
		if(curimg.empty())
			continue;

		//visualsearch::ImgVisualizer::DrawImgWins("b3d", curimg, rawgtwins[img_fns[i].filename]);
		//waitKey(0);

		// resize
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(curimg.cols, curimg.rows), 300, newsz);
		//resize(curimg, curimg, newsz);

		// get objectness windows
		vector<ImgWin> objboxes;
		detector.GetObjectsFromBing(curimg, objboxes, 1000);
		//visualsearch::ImgVisualizer::DrawImgWins("objectness", curimg, objboxes);
		
		// rank
		vector<ImgWin> salboxes = objboxes;
		saldet.Init(curimg);
		saldet.RankWins(salboxes);
		//visualsearch::ImgVisualizer::DrawImgWins("sal", curimg, salboxes);
		//waitKey(0);

		// add to collection
		objdetwins.push_back(objboxes);
		saldetwins.push_back(salboxes);
		gtwins.push_back(rawgtwins[img_fns[i].filename]);
		
		cout<<"Finish detection on "<<i<<"/"<<img_fns.size()<<endl;
	}

	// evaluation
	WindowEvaluator eval;
	vector<Point2f> objprvals, salprvals;
	int topnum[] = {1, 5, 10, 50, 100, 200, 500, 800, 1000};
	for(int i=0; i<9; i++)
	{
		Point2f curpr = eval.ComputePR(objdetwins, gtwins, topnum[i]);
		objprvals.push_back(curpr);
		curpr = eval.ComputePR(saldetwins, gtwins, topnum[i]);
		salprvals.push_back(curpr);
	}
	
	// save to file
	ofstream out1("objpr.txt");
	for (size_t i=0; i<objprvals.size(); i++) out1<<objprvals[i].x<<" "<<objprvals[i].y<<endl;
	ofstream out2("salpr.txt");
	for (size_t i=0; i<salprvals.size(); i++) out2<<salprvals[i].x<<" "<<salprvals[i].y<<endl;

	cout<<"Finish evaluation"<<endl;

}