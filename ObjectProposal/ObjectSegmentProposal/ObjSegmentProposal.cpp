#include "ObjSegmentProposal.h"

namespace objectproposal
{
	ObjSegmentProposal::ObjSegmentProposal(void)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	bool ObjSegmentProposal::Run(const Mat& cimg, const Mat& dmap, int topK, vector<SuperPixel>& res)
	{
		// get candidates
		vector<SuperPixel> init_res;
		iter_segmentor.Init();
		iter_segmentor.verbose = false;
		iter_segmentor.Run(cimg, dmap, init_res);

		// rank
		vector<int> rank_ids;
		seg_ranker.RankSegments(cimg, dmap, init_res, visualsearch::processors::attention::	SEG_RANK_CC, rank_ids);

		// filter results
		res.clear();
		res.reserve(topK);
		for (int i=0; i<MIN(topK, rank_ids.size()); i++)
		{
			res.push_back(init_res[rank_ids[i]]);
		}

		return true;
	}

	bool ObjSegmentProposal::VisProposals(const Mat& cimg, const vector<SuperPixel>& res)
	{
		char str[30];
		for (size_t i=0; i<res.size(); i++)
		{
			sprintf_s(str, "%d proposal", i);
			Mat dispimg(cimg.rows, cimg.cols, CV_8UC3);
			dispimg.setTo(Vec3b(0,0,0));
			cimg.copyTo(dispimg, res[i].mask);
			rectangle(dispimg, res[i].box, CV_RGB(255, 0, 0));
			imshow(str, dispimg);
			waitKey(0);
		}

		return true;
	}
}


