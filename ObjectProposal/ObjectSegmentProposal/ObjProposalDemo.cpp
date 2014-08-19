#include "ObjProposalDemo.h"


ObjProposalDemo::ObjProposalDemo()
{
}


bool ObjProposalDemo::RunObjWinProposal()
{
	//KinectDataMan kinectDM;
	visualsearch::io::OpenCVCameraIO cam;
	if( !cam.InitCamera() )
		return false;

	//if( !kinectDM.InitKinect() )
		//return false;

	visualsearch::processors::attention::BingObjectness bing;
	visualsearch::processors::attention::ObjectRanker ranker;
	visualsearch::visualization::ImgVisualizer imgvis;

	while(1)
	{
		Mat cimg, dmap;
		cam.QueryNextFrame(visualsearch::io::STREAM_COLOR, cimg);
		//kinectDM.GetColorDepth(cimg, dmap);

		// resize image
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
		resize(cimg, cimg, newsz);

		// get objects
		vector<ImgWin> objwins, salwins;
		if( !bing.GetProposals(cimg, objwins, 1000) )
			continue;

		// rank
		vector<int> sorted_ids;
		ranker.RankWindowsBySaliency(cimg, objwins, sorted_ids);

		vector<ImgWin> drawwins;
		for (size_t i=0; i<MIN(sorted_ids.size(), 10); i++)
			drawwins.push_back(objwins[sorted_ids[i]]);

		imshow("input", cimg);
		imgvis.DrawCroppedWins("obj", cimg, drawwins, 5);

		if( waitKey(10) == 'q' )
			break;
	}


	return true;
}


bool ObjProposalDemo::RunSaliency(visualsearch::processors::attention::SaliencyType saltype)
{
	//KinectDataMan kinectDM;
	visualsearch::io::OpenCVCameraIO cam;
	if( !cam.InitCamera() )
		return false;

	//if( !kinectDM.InitKinect() )
		//return false;

	visualsearch::processors::attention::SaliencyComputer salcomputer;

	while(1)
	{
		Mat cimg, dmap;
		cam.QueryNextFrame(visualsearch::io::STREAM_COLOR, cimg);
		//kinectDM.GetColorDepth(cimg, dmap);

		// resize image
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
		resize(cimg, cimg, newsz);

		Mat salmap;
		salcomputer.ComputeSaliencyMap(cimg, saltype, salmap);

		imshow("color", cimg);
		imshow("sal", salmap);
		if( waitKey(10) == 'q' )
			break;
	}

	return true;
}