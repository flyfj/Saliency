#include "ObjProposalDemo.h"


ObjProposalDemo::ObjProposalDemo()
{
	frameid = 0;
	DATADIR = "e:\\res\\kinectvideos\\1\\";
}

//////////////////////////////////////////////////////////////////////////

bool ObjProposalDemo::RunVideoDemo(SensorType stype, DemoType dtype)
{
	bool tosave = true;
	visualsearch::io::camera::OpenCVCameraIO cam;
	if(stype == SENSOR_CAMERA)
	{
		if( !cam.InitCamera() )
			return false;
	}
	KinectDataMan kinect;
	if(stype == SENSOR_KINECT)
	{
		if( !kinect.InitKinect() )
			return false;
	}

	//if( !kinectDM.InitKinect() )
	//return false;

	char str[100];
	frameid = 0;

	while(1)
	{
		Mat cimg, dmap;
		if(stype == SENSOR_CAMERA) {
			if( !cam.QueryNextFrame(visualsearch::io::camera::STREAM_COLOR, cimg) )
				continue;
		}
		if(stype == SENSOR_KINECT) {
			if( !kinect.GetColorDepth(cimg, dmap) )
				continue;
		}

		// downsample cimg to have same size as dmap
		//resize(cimg, cimg, Size(cimg.cols/2, cimg.rows/2));
		// show input
		imshow("color", cimg);
		imshow("depth", dmap);
		//imgvis.DrawFloatImg("depth", dmap, Mat());

		if(tosave) {
			sprintf_s(str, "frame_%d.jpg", frameid);
			imwrite(DATADIR+string(str), cimg);
			sprintf_s(str, "frame_%d_d.png", frameid);
			imwrite(DATADIR+string(str), dmap);
		}
		
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
	visualsearch::common::tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 320, newsz);
	resize(cimg, cimg, newsz);
	if(!dmap.empty()) resize(dmap, dmap, newsz);

	// propose
	vector<SuperPixel> sps;
	seg_proposal.Run(cimg, dmap, 5, sps);

	return true;
	
	// display results
	Mat oimg;
	imgvis.DrawShapes(cimg, sps);
	/*char str[30];
	sprintf_s(str, "%d_0.jpg", frameid);
	imwrite(DATADIR + string(str), cimg);
	imgvis.DrawWinsOnImg("input", cimg, drawwins, oimg);
	sprintf_s(str, "%d_1.png", frameid);
	dmap.copyTo(oimg);
	imwrite(DATADIR + string(str), oimg);
	imgvis.DrawCroppedWins("obj", cimg, drawwins, 5, oimg);
	sprintf_s(str, "%d_2.jpg", frameid);
	imwrite(DATADIR + string(str), oimg);*/
	
	// check lock file
	/*string lockfn = DATADIR + "demo.lock";
	ifstream in(lockfn);
	if( in.is_open() )
		return true;*/

	// save object
	//for (size_t i=0; i<drawwins.size(); i++)
	//{
	//	// color
	//	sprintf_s(str, "obj_%d_%d_c.png", frameid, i);
	//	imwrite(DATADIR+string(str), cimg(drawwins[i]));
	//	if( dmap.empty() )
	//		continue;
	//	// depth
	//	sprintf_s(str, "obj_%d_%d_d.png", frameid, i);
	//	imwrite(DATADIR+string(str), dmap(drawwins[i]));
	//}

	// write lock file
	/*ofstream out(lockfn);
	out.close();*/

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