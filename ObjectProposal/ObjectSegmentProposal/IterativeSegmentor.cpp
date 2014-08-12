//////////////////////////////////////////////////////////////////////////


#include "IterativeSegmentor.h"

namespace objectproposal
{
	IterativeSegmentor::IterativeSegmentor(void)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IterativeSegmentor::Init()
	{
		// do segmentation
		img_segmentor.m_dMinArea = 200;
		img_segmentor.m_dSmoothSigma = 0.5f;
		img_segmentor.m_dThresholdK = 50.f;

		cur_seg_id = 0;
	}

	bool IterativeSegmentor::ComputeSPFeatures(const Mat& cimg, SuperPixel& sp)
	{
		sp.meancolor = mean(cimg, sp.mask);
		return true;
	}

	float IterativeSegmentor::ComputeSPDist(const SuperPixel& sp1, const SuperPixel& sp2)
	{
		float dist = 0;
		for (int i=0; i<3; i++)
			dist += (sp1.meancolor.val[i]-sp2.meancolor.val[i])*(sp1.meancolor.val[i]-sp2.meancolor.val[i]);
		dist = sqrt(dist);
		return dist;
	}

	bool IterativeSegmentor::Run(const Mat& cimg)
	{
		// oversegmentation
		img_segmentor.DoSegmentation(cimg);
		imshow("input", cimg);
		imshow("seg", img_segmentor.m_segImg);
		waitKey(10);
		vector<SuperPixel>& res_sps = img_segmentor.superPixels;
		Mat adjMat;
		img_segmentor.ComputeAdjacencyMat(res_sps, adjMat);

		// add to collection
		sps.clear();
		for (size_t i=0; i<res_sps.size(); i++) 
		{
			sps[cur_seg_id] = res_sps[i];
			ComputeSPFeatures(cimg, sps[cur_seg_id]);
			cur_seg_id++;
		}

		// create initial sp pairs
		sp_pairs.clear();
		for (int r=0; r<adjMat.rows; r++)
			for(int c=r+1; c<adjMat.cols; c++)
				if(adjMat.at<uchar>(r,c) > 0)
				{
					float spdist = ComputeSPDist(sps[r], sps[c]);
					sp_pairs[spdist] = Point(r, c);
				}
		
		// start merging process
		while(!sp_pairs.empty())
		{
			DoMergeIteration(cimg, true);
		}

		return true;
	}

	bool IterativeSegmentor::DoMergeIteration(const Mat& cimg, bool verbose)
	{
		// select the best pair
		Point bestpair = sp_pairs.begin()->second;
		sp_pairs.erase(sp_pairs.begin());
		// create new sp
		SuperPixel newsp;
		newsp.mask = sps[bestpair.x].mask | sps[bestpair.y].mask;
		ComputeSPFeatures(cimg, newsp);
		sps[cur_seg_id] = newsp;
		if(verbose)
		{
			imshow("new merge", newsp.mask);
			waitKey(0);
		}
		// add new merge pairs and remove old pairs
		map<int, int> mergedPairs;	// save already merged pairs
		map<float, Point> newpairs;
		for (SimPair::iterator pi=sp_pairs.begin(); pi!=sp_pairs.end(); pi++)
		{
			// for any pair containing one of members in to merge pair
			Point nextPair;
			nextPair.y = cur_seg_id;
			if(pi->second.x == bestpair.x || pi->second.x == bestpair.y)
				nextPair.x = pi->second.y;
			if(pi->second.y == bestpair.x || pi->second.y == bestpair.y)
				nextPair.x = pi->second.x;
			// check if the merge has been done
			if(mergedPairs[nextPair.x] == nextPair.y)
				continue;
			// compute distance
			float curdist = ComputeSPDist(sps[nextPair.x], sps[nextPair.y]);
			newpairs[curdist] = nextPair;
			mergedPairs[nextPair.x] = nextPair.y;
		}
		// remove pairs containing merged sps
		bool hasErased = false;
		for (SimPair::iterator pi=sp_pairs.begin(); pi!=sp_pairs.end(); (hasErased? pi:pi++))
		{
			hasErased = false;
			if(pi->second.x == bestpair.x || pi->second.x == bestpair.y 
				|| pi->second.y == bestpair.x || pi->second.y == bestpair.y)
			{	
				sp_pairs.erase(pi);
				hasErased = true;
			}
		}
		// add new pairs
		for (SimPair::iterator pi=newpairs.begin(); pi!=newpairs.end(); pi++)
			sp_pairs[pi->first] = pi->second;

		cur_seg_id++;

		return true;
	}


	


}

