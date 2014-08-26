#include "ObjProposalDemo.h"


ObjProposalDemo::ObjProposalDemo()
{
	frameid = 0;
}

//////////////////////////////////////////////////////////////////////////

bool ObjProposalDemo::RunVideoDemo(DemoType dtype)
{
	//KinectDataMan kinectDM;
	visualsearch::io::camera::OpenCVCameraIO cam;
	if( !cam.InitCamera() )
		return false;

	//if( !kinectDM.InitKinect() )
	//return false;

	frameid = 0;

	while(1)
	{
		Mat cimg, dmap;
		//kinectDM.GetColorDepth(cimg, dmap);
		if( !cam.QueryNextFrame(visualsearch::io::camera::STREAM_COLOR, cimg) )
			continue;

		frameid++;

		if(dtype == DEMO_OBJECT_WIN)
			RunObjWinProposal(cimg, dmap);
		if(dtype == DEMO_OBJECT_SEG)
			RunObjSegProposal(cimg, dmap);
		if(dtype == DEMO_SAL)
			RunSaliency(cimg, dmap, visualsearch::processors::attention::SAL_HC);

		if( waitKey(10) == 'q' )
			break;
	}

	return true;
}

bool ObjProposalDemo::RunObjSegProposal(Mat& cimg, Mat& dmap)
{
	// resize image
	Size newsz;
	visualsearch::common::tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);

	vector<SuperPixel> sps;
	//iterSegmentor.verbose = true;
	iterSegmentor.Run(cimg, dmap, sps);

	// rank
	vector<ImgWin> objwins;
	for (size_t i=0; i<sps.size(); i++)
	{
		ImgWin curwin = ImgWin(sps[i].box.x, sps[i].box.y, sps[i].box.width, sps[i].box.height);
		if(curwin.area()*1.0f / (cimg.rows*cimg.cols) < 0.2)
			continue;

		objwins.push_back(curwin);
	}
	vector<int> sorted_ids;
	ranker.RankWindowsBySaliency(cimg, objwins, sorted_ids);

	// nms
	vector<ImgWin> drawwins = visualsearch::processors::nms(objwins, 0.6f);
	drawwins.resize(MIN(drawwins.size(), 10));

	Mat oimg;
	char str[30];
	imgvis.DrawWinsOnImg("input", cimg, drawwins);
	sprintf_s(str, "e:\\res\\objectness\\%d_1.jpg", frameid);
	imwrite(str, cimg);
	imgvis.DrawCroppedWins("obj", cimg, drawwins, 5, oimg);
	sprintf_s(str, "e:\\res\\objectness\\%d_2.jpg", frameid);
	imwrite(str, oimg);


	return true;
}

bool ObjProposalDemo::RunObjWinProposal(Mat& cimg, Mat& dmap)
{
	// resize image
	Size newsz;
	tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);

	/*Mat salmap;
	salcomputer.ComputeSaliencyMap(cimg, SAL_HC, salmap);
	salmap.convertTo(salmap, CV_8U, 255);
	cvtColor(salmap, salmap, CV_GRAY2BGR);
	imshow("salcolor", salmap);
	waitKey(10);
	*/

	// get objects
	vector<ImgWin> objwins, salwins;
	if( !bing.GetProposals(cimg, objwins, 1000) )
		return false;

	// rank
	vector<int> sorted_ids;
	ranker.RankWindowsBySaliency(cimg, objwins, sorted_ids);

	// nms
	vector<ImgWin> drawwins = visualsearch::processors::nms(objwins, 0.4f);
	drawwins.resize(MIN(drawwins.size(), 10));

	//for (size_t i=0; i<MIN(sorted_ids.size(), 10); i++)
		//drawwins.push_back(objwins[sorted_ids[i]]);
	//objwins.erase(objwins.begin()+10, objwins.end());
	
	Mat oimg;
	char str[30];
	imgvis.DrawWinsOnImg("input", cimg, drawwins);
	sprintf_s(str, "e:\\res\\objectness\\%d_1.jpg", frameid);
	imwrite(str, cimg);
	imgvis.DrawCroppedWins("obj", cimg, drawwins, 5, oimg);
	sprintf_s(str, "e:\\res\\objectness\\%d_2.jpg", frameid);
	imwrite(str, oimg);

	return true;
}


bool ObjProposalDemo::RunSaliency(Mat& cimg, Mat& dmap, visualsearch::processors::attention::SaliencyType saltype)
{
	// resize image
	Size newsz;
	tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
	resize(cimg, cimg, newsz);

	Mat salmap;
	salcomputer.ComputeSaliencyMap(cimg, saltype, salmap);

	imshow("color", cimg);
	imshow("sal", salmap);

	return true;
}