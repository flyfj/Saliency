#include "ObjProposalDemo.h"


ObjProposalDemo::ObjProposalDemo()
{
}

bool ObjProposalDemo::RunObjWinProposal()
{
	KinectDataMan kinectDM;

	if( !kinectDM.InitKinect() )
		return false;

	visualsearch::processors::attention::BingObjectness bing;
	visualsearch::processors::attention::ObjectRanker ranker;

	while(1)
	{
		Mat cimg, dmap;
		kinectDM.GetColorDepth(cimg, dmap);

		// resize image
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
		resize(cimg, cimg, newsz);

		// get objects
		vector<ImgWin> objwins, salwins;
		if( !bing.GetProposals(cimg, objwins, 500) )
			continue;

		// rank
		vector<int> sorted_ids;
		ranker.RankWindowsBySaliency(cimg, objwins, sorted_ids);

		// show results
		for (size_t i=0; i<sorted_ids.size(); i++)
		{
			if(i > 10)
				break;
			rectangle(cimg, objwins[i], CV_RGB(255, 0, 0));
		}

		imshow("img", cimg);
		if( waitKey(0) == 'q' )
			break;
	}


	return true;
}


bool ObjProposalDemo::RunSaliency(visualsearch::processors::attention::SaliencyType saltype)
{
	KinectDataMan kinectDM;

	if( !kinectDM.InitKinect() )
		return false;

	visualsearch::processors::attention::SaliencyComputer salcomputer;

	while(1)
	{
		Mat cimg, dmap;
		kinectDM.GetColorDepth(cimg, dmap);

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