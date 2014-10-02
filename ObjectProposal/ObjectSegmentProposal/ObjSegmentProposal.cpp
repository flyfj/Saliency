#include "ObjSegmentProposal.h"

namespace objectproposal
{
	ObjSegmentProposal::ObjSegmentProposal(void)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	bool ObjSegmentProposal::Compute3DDistMap(const Mat& dmap, Mat& distmap)
	{
		Mat map3d;
		visualsearch::common::tools::RGBDTools::KinectDepthTo3D(dmap, map3d);
		distmap.create(dmap.rows, dmap.cols, CV_32F);
		distmap.setTo(0);

		for(int r=1; r<map3d.rows-1; r++) {
			for(int c=1; c<map3d.cols-1; c++) {
				Vec3f leftpt = map3d.at<Vec3f>(r, c-1);
				Vec3f rightpt = map3d.at<Vec3f>(r, c+1);
				Vec3f toppt = map3d.at<Vec3f>(r-1, c);
				Vec3f bottompt = map3d.at<Vec3f>(r+1, c);
				Vec3f curpt = map3d.at<Vec3f>(r, c);
				float maxdist = 0;
				float dist = 0;
				for(int i=0; i<3; i++)
					dist += (curpt.val[i]-leftpt.val[i])*(curpt.val[i]-leftpt.val[i]);
				dist = sqrt(dist);
				maxdist = (dist > maxdist? dist: maxdist);
				
				dist = 0;
				for(int i=0; i<3; i++)
					dist += (curpt.val[i]-rightpt.val[i])*(curpt.val[i]-rightpt.val[i]);
				dist = sqrt(dist);
				maxdist = (dist > maxdist? dist: maxdist);
				
				dist = 0;
				for(int i=0; i<3; i++)
					dist += (curpt.val[i]-toppt.val[i])*(curpt.val[i]-toppt.val[i]);
				dist = sqrt(dist);
				maxdist = (dist > maxdist? dist: maxdist);
				
				dist = 0;
				for(int i=0; i<3; i++)
					dist += (curpt.val[i]-bottompt.val[i])*(curpt.val[i]-bottompt.val[i]);
				dist = sqrt(dist);
				maxdist = (dist > maxdist? dist: maxdist);

				distmap.at<float>(r, c) = maxdist;
			}
		}

		visualsearch::tools::ImgVisualizer::DrawFloatImg("dmap", dmap, Mat());
		visualsearch::tools::ImgVisualizer::DrawFloatImg("3dmap", distmap, Mat());
		waitKey(0);

		return true;
	}

	bool ObjSegmentProposal::Run(const Mat& cimg, const Mat& dmap, int topK, vector<SuperPixel>& res)
	{
		// get candidates
		iter_segmentor.merge_feat_types = visualsearch::processors::segmentation::SP_COLOR;
		iter_segmentor.Init(cimg, dmap);
		iter_segmentor.verbose = false;
		iter_segmentor.Run();
		const vector<SuperPixel>& res_sps = iter_segmentor.sps;

		// rank
		vector<int> rank_ids;
		seg_ranker.RankSegments(cimg, dmap, res_sps, visualsearch::processors::attention::SEG_RANK_SALIENCY, rank_ids);

		// filter results
		/*vector<int> valid_ids;
		for (size_t i=0; i<rank_ids.size(); i++) {
		if(res_sps[rank_ids[i]].area*1.0f / (cimg.rows*cimg.cols) < 0.15)
		continue;

		valid_ids.push_back(rank_ids[i]);
		}*/

		res.clear();
		res.reserve(topK);
		for (int i=0; i<MIN(topK, rank_ids.size()); i++) {
			res.push_back(res_sps[rank_ids[i]]);
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


