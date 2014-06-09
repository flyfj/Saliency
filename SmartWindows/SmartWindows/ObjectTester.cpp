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

	NYUDepth2DataMan nyudata;
	Berkeley3DDataManager berkeleydata;

	if(dbname == DB_NYU2_RGBD)
	{
		nyudata.GetImageList(img_fns);
		nyudata.GetDepthmapList(dmap_fns);
		if(img_fns.size() != dmap_fns.size())
			return;
		nyudata.LoadGTWins(img_fns, rawgtwins);
	}
	if(dbname == DB_BERKELEY3D)
	{
		berkeleydata.GetImageList(img_fns);
		berkeleydata.GetDepthmapList(dmap_fns);
		if(img_fns.size() != dmap_fns.size())
			return;
		berkeleydata.LoadGTWins(img_fns, rawgtwins);
	}

	GenericObjectDetector detector;
	if( !detector.InitBingObjectness() )
		return;

	SalientRegionDetector saldet;
	DepthSaliency depth_sal;
	vector<vector<ImgWin>> objdetwins(img_fns.size()), saldetwins(img_fns.size()), depthdetwins;
	vector<vector<ImgWin>> gtwins(img_fns.size());

#pragma omp parallel for
	for (int i=0; i<img_fns.size(); i++)
	{
		// read image
		Mat curimg = imread(img_fns[i].filepath);
		if(curimg.empty())
			continue;

		// read depth
		Mat curdmap;
		if(dbname == DB_NYU2_RGBD)
			nyudata.LoadDepthData(dmap_fns[i].filepath, curdmap);
		if(dbname == DB_BERKELEY3D)
			berkeleydata.LoadDepthData(dmap_fns[i].filepath, curdmap);

		// normalize to image
		//normalize(curdmap, curdmap, 0, 255, NORM_MINMAX);
		//curdmap.convertTo(curdmap, CV_8U);
		//cvtColor(curdmap, curdmap, CV_GRAY2BGR);
		//imshow("depthimg", curdmap);
		//waitKey(10);

		//visualsearch::ImgVisualizer::DrawImgWins("b3d", curimg, rawgtwins[img_fns[i].filename]);
		//waitKey(0);

		// resize
		Size newsz;
		//ToolFactory::compute_downsample_ratio(Size(curimg.cols, curimg.rows), 300, newsz);
		//resize(curimg, curimg, newsz);

		// get objectness windows
		vector<ImgWin> objboxes;
		detector.GetObjectsFromBing(curimg, objboxes, 1000);
		//visualsearch::ImgVisualizer::DrawImgWins("objectness", curimg, objboxes);
		
		// rank
		vector<ImgWin> salboxes = objboxes;
		depth_sal.RankWins(curdmap, salboxes);
		/*saldet.g_para.segThresholdK = 200;
		saldet.Init(curdmap);
		saldet.RankWins(salboxes);*/
		//visualsearch::ImgVisualizer::DrawImgWins("sal", curimg, salboxes);
		//waitKey(0);

		// add to collection
		objdetwins[i] = objboxes;
		saldetwins[i] = salboxes;
		gtwins[i] = rawgtwins[img_fns[i].filename];
		
		cout<<"Finish detection on "<<i<<"/"<<img_fns.size()<<endl;
	}

	// evaluation
	WindowEvaluator eval;
	vector<Point2f> objprvals, salprvals, depthprvals;
	int topnum[] = {1, 5, 10, 50, 100, 200, 500, 800, 1000};
	for(int i=0; i<9; i++)
	{
		Point2f curpr = eval.ComputePR(objdetwins, gtwins, topnum[i]);
		objprvals.push_back(curpr);
		curpr = eval.ComputePR(saldetwins, gtwins, topnum[i]);
		salprvals.push_back(curpr);
	}
	
	// save to file
	ofstream out1("b3d_objpr.txt");
	for (size_t i=0; i<objprvals.size(); i++) out1<<objprvals[i].x<<" "<<objprvals[i].y<<endl;
	ofstream out2("b3d_dvarpr.txt");
	for (size_t i=0; i<salprvals.size(); i++) out2<<salprvals[i].x<<" "<<salprvals[i].y<<endl;

	cout<<"Finish evaluation"<<endl;

}


void ObjectTester::RunVideoDemo()
{
	GenericObjectDetector detector;

	KinectDataMan kinectDM;

	if( !kinectDM.InitKinect() )
		return;

	// start fetching stream data
	while(1)
	{
		Mat cimg, dmap;
		if( !kinectDM.GetColorDepth(cimg, dmap) )
			return;

		// resize image
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
		resize(cimg, cimg, newsz);
		resize(dmap, dmap, newsz);

		// get objects
		vector<ImgWin> wins;
		if( !detector.ProposeObjects(cimg, dmap, wins) )
			continue;

		//////////////////////////////////////////////////////////////////////////
		// draw best k windows
		int topK = MIN(6, wins.size());
		int objimgsz = newsz.height / topK;
		int canvas_h = newsz.height;
		int canvas_w = newsz.width + 10 + objimgsz;
		Mat canvas(canvas_h, canvas_w, CV_8UC3);
		canvas.setTo(Vec3b(0,0,0));
		// get top windows
		vector<Mat> detimgs(topK);
		for (int i=0; i<topK; i++)
		{
			cimg(wins[i]).copyTo(detimgs[i]);
			resize(detimgs[i], detimgs[i], Size(objimgsz, objimgsz));
		}

		// draw boxes on input
		for(int i=0; i<topK; i++)
		{
			Scalar color(rand()%255, rand()%255, rand()%255);
			rectangle(cimg, wins[i], color);
		}
		circle(cimg, Point(cimg.cols/2, cimg.rows/2), 2, CV_RGB(255,0,0));
		// copy input image
		cimg.copyTo(canvas(Rect(0, 0, cimg.cols, cimg.rows)));

		// copy subimg
		for (int i=0; i<detimgs.size(); i++)
		{
			Rect curbox(cimg.cols+10, i*objimgsz, objimgsz, objimgsz);
			detimgs[i].copyTo(canvas(curbox));
		}

		resize(canvas, canvas, Size(canvas.cols*3, canvas.rows*3));
		imshow("object proposals", canvas);
		if( waitKey(10) == 'q' )
			break;
	}
}