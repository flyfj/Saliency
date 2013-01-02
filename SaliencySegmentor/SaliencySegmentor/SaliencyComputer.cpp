#include "SaliencyComputer.h"


namespace Saliency
{

	SaliencyComputer::SaliencyComputer(void)
	{
	}


	SaliencyComputer::~SaliencyComputer(void)
	{
	}


	//////////////////////////////////////////////////////////////////////////

	// compute spatial distance
	float SaliencyComputer::Point2RectDistance(int x, int y, const Rect& box)
	{
		if (box.contains(Point(x,y))) return 0;

		if(x<box.x)
		{
			if(y<box.y)
				return l2_dist(x, y, box.x, box.y);
			else if(y<box.br().y)
				return (box.x-x);
			else
				return l2_dist(x, y, box.x, box.br().y);
		}
		else if(x<box.br().x)
		{
			if(y<box.y)
				return (box.y-y);
			else if(y>box.br().y)
				return (y-box.br().y);
			else
				return 0;
		}
		else
		{
			if(y<box.y)
				return l2_dist(x, y, box.br().x, box.y);
			else if(y<box.br().y)
				return (x-box.br().x);
			else
				return l2_dist(x, y, box.br().x, box.br().y);
		}
	}

	float SaliencyComputer::HausdorffDist(Rect a, Rect b)
	{
		// compute approximate Hausdorff distance
		// a->b
		float mindist1 = 999999999.f;
		Rect box1 = a;
		Rect box2 = b;

		vector<Point> points(4);
		points[0] = Point(box1.x, box1.y);
		points[1] = Point(box1.x, box1.br().y);
		points[2] = Point(box1.br().x, box1.y);
		points[3] = Point(box1.br().x, box1.br().y);

		for(size_t id = 0; id < 4; id++)
		{
			float dist = Point2RectDistance(points[id].x, points[id].y, box2);
			if(dist < mindist1)
				mindist1 = dist;
		}

		// b->a
		float mindist2 = 999999999.9f;
		points[0] = Point(box2.x, box2.y);
		points[1] = Point(box2.x, box2.br().y);
		points[2] = Point(box2.br().x, box2.y);
		points[3] = Point(box2.br().x, box2.br().y);

		for(size_t id = 0; id < 4; id++)
		{
			float dist = Point2RectDistance(points[id].x, points[id].y, box1);
			if(dist < mindist2)
				mindist2 = dist;
		}

		return max(mindist1, mindist2);
	}


	//////////////////////////////////////////////////////////////////////////

	bool SaliencyComputer::InitCompositionFeature(vector<SegSuperPixelFeature>& prim_sp_feats)
	{

		for (size_t i=0; i<prim_sp_feats.size(); i++)
		{
			// boundary area extension
			prim_sp_feats[i].compose_feat.extendedArea = prim_sp_feats[i].area;
			if(prim_sp_feats[i].bnd_pixels > 0)
				prim_sp_feats[i].compose_feat.extendedArea *= 
				(1 + 2*(float)prim_sp_feats[i].bnd_pixels / prim_sp_feats[i].perimeter);
		}

		// compute pair-wise composition cost
		float maxAppdist = 0, maxSpadist = 0;
		for (int curIdx = 0; curIdx < prim_sp_feats.size(); curIdx++)
		{	
			// allocate space
			prim_sp_feats[curIdx].compose_feat.pairs.reserve(prim_sp_feats.size());

			// temp object
			TPair pair;
			// compute with each other prim sp
			for (int nextIdx = 0; nextIdx < prim_sp_feats.size(); nextIdx++)
			{			
				pair.id = nextIdx;

				// appearance distance
				pair.appdist = SegSuperPixelFeature::FeatureIntersectionDistance(
					prim_sp_feats[curIdx], prim_sp_feats[nextIdx]);

				if (pair.appdist > maxAppdist)
					maxAppdist = pair.appdist;

				// spatial distance
				pair.spadist = HausdorffDist(prim_sp_feats[curIdx].box, prim_sp_feats[nextIdx].box);

				if (pair.spadist > maxSpadist)
					maxSpadist = pair.spadist;

				// add to list
				prim_sp_feats[curIdx].compose_feat.pairs.push_back(pair);
			}
		}

		// normalize
		for (int n = 0; n < prim_sp_feats.size(); n++)
		{
			for(size_t i = 0; i < prim_sp_feats[n].compose_feat.pairs.size(); i++)
			{			
				prim_sp_feats[n].compose_feat.pairs[i].appdist /= maxAppdist;
				prim_sp_feats[n].compose_feat.pairs[i].spadist /= maxSpadist;
			}
		}
		// sort all lists
		for (int n = 0; n < prim_sp_feats.size(); n++)
		{
			SegSuperPixelComposeFeature& curfeat = prim_sp_feats[n].compose_feat;
			// compute distance(similarity) with other segments
			for(size_t pi = 0; pi < curfeat.pairs.size(); pi++)
			{			
				float weight = curfeat.pairs[pi].spadist;

				// set saliency value for each superpixel
				curfeat.pairs[pi].saliency = (1-weight)*curfeat.pairs[pi].appdist + weight*1;
			}

			// sort by saliency
			sort(curfeat.pairs.begin(), curfeat.pairs.end(), TPair::comp_by_saliency);
		}

		return true;

	}


	float SaliencyComputer::ComputeSegmentSaliency(
		const Mat& img, 
		const SegSuperPixelFeature& sp_feat, 
		vector<SegSuperPixelFeature>& prim_sp_feats, 
		SaliencyType type)
	{

		if( type == Composition )
		{
			return Compose(sp_feat, prim_sp_feats);
		}
		if( type == CenterSurroundHistogramContrast )
		{

			if(lab_img.empty())
				cvtColor(img, lab_img, CV_BGR2Lab);

			// ignore too big segment
			///*if(sp_feat.area >= 0.6*img.rows*img.cols)
			//	return -1;*/

			// define surround context
			// bounding box with 2X width and height
			Rect segment_box = sp_feat.box;
			Point tl_pts(segment_box.tl().x-segment_box.width, segment_box.tl().y-segment_box.height);
			tl_pts.x = MAX(0, tl_pts.x);
			tl_pts.y = MAX(0, tl_pts.y);
			Point br_pts(segment_box.br().x+segment_box.width, segment_box.br().y+segment_box.height);
			br_pts.x = MIN(br_pts.x, img.cols-1);
			br_pts.y = MIN(br_pts.y, img.rows-1);
			Rect context_box(tl_pts, br_pts);
			context_box = Rect(0, 0, img.cols, img.rows);

			// show boxes
			Mat temp_img = img.clone();
			rectangle(temp_img, segment_box, CV_RGB(0,255,0), 1);
			rectangle(temp_img, context_box, CV_RGB(255,255,0), 1);
			imshow("saliency", temp_img);
			waitKey(10);

			// compute context feature
			int count = 0;
			vector<float> context_feat(quantBins[0]+quantBins[1]+quantBins[2], 0);
			for(int y=context_box.tl().y; y<context_box.br().y; y++)
			{
				for(int x=context_box.tl().x; x<context_box.br().x; x++)
				{
					if( SegSuperPixelFeature::InsideSegment(Point(x,y), sp_feat) )
						continue;

					/*Vec3b val = img.at<Vec3b>(y,x);
					float b = val.val[0];
					float g = val.val[1];
					float r = val.val[2];
					int bbin = (int)(b/(255.f/quantBins[0]));
					bbin = ( bbin > quantBins[0]-1? quantBins[0]-1: bbin );
					int gbin = (int)(g/(255.f/quantBins[1]));
					gbin = ( gbin > quantBins[1]-1? quantBins[1]-1: gbin );
					int rbin = (int)(r/(255.f/quantBins[2]));
					rbin = ( rbin > quantBins[2]-1? quantBins[2]-1: rbin );

					context_feat[bbin]++;
					context_feat[quantBins[0]+gbin]++;
					context_feat[quantBins[0]+quantBins[1]+rbin]++;*/

					Vec3b val = lab_img.at<Vec3b>(y,x);
					float l = val.val[0];
					float a = val.val[1];
					float b = val.val[2];
					int lbin = (int)(l/(255.f/quantBins[0]));
					lbin = ( lbin > quantBins[0]-1? quantBins[0]-1: lbin );
					int abin = (int)(a/(255.f/quantBins[1]));
					abin = ( abin > quantBins[1]-1? quantBins[1]-1: abin );
					int bbin = (int)(b/(255.f/quantBins[2]));
					bbin = ( bbin > quantBins[2]-1? quantBins[2]-1: bbin );

					context_feat[lbin]++;
					context_feat[quantBins[0]+abin]++;
					context_feat[quantBins[0]+quantBins[1]+bbin]++;
					count+=3;
				}
			}

			// do normalization
			vector<float> feat1(context_feat.size(), 0);	// segment
			vector<float> feat2(context_feat.size(), 0);	// context
			for(size_t i=0; i<sp_feat.feat.size(); i++)
			{
				feat1[i] = sp_feat.feat[i] / (sp_feat.area*3);
				feat2[i] = context_feat[i] / count;
			}

			// compute distance
			float dist = 0;
			for(size_t i=0; i<feat1.size(); i++)
				dist += (feat1[i]-feat2[i])*(feat1[i]-feat2[i]);
			dist = sqrt(dist);

			if(dist > 1)
				cout<<"error"<<endl;

			return dist;

		}


	}


	float SaliencyComputer::Compose(const SegSuperPixelFeature& sp_feat, vector<SegSuperPixelFeature>& prim_sp_feats)
	{
		
		vector<SegSuperPixelComposeFeature*> innerSegs;
		innerSegs.reserve(prim_sp_feats.size());


		// only change composition related data (area)
		for(size_t i=0; i<prim_sp_feats.size(); i++)
		{
			SegSuperPixelFeature& cur_feat = prim_sp_feats[sp_feat.components[i]];
			if(sp_feat.components[i])
			{
				// inner sp
				cur_feat.compose_feat.leftInnerArea = MAX(0, 2*cur_feat.area-cur_feat.compose_feat.extendedArea);
				cur_feat.compose_feat.leftOuterArea = 0;
			}
			else
			{
				// outer sp
				cur_feat.compose_feat.leftInnerArea = 0;
				cur_feat.compose_feat.leftOuterArea = cur_feat.compose_feat.extendedArea;
			}

			// need to compose
			if(cur_feat.compose_feat.leftInnerArea > 0)
			{
				cur_feat.compose_feat.dist_to_win = l2_dist(cur_feat.centroid, sp_feat.centroid);
				innerSegs.push_back( &cur_feat.compose_feat );
			}
		}



		/************************************************************************/
		/*   do composition
		/************************************************************************/

		sort(innerSegs.begin(), innerSegs.end(), SegSuperPixelComposeFeature::comp_by_dist);
		float rate = 0;	// compute weight ratio to penalize background window
		float winscore = 0;

		for (size_t i = 0; i < innerSegs.size(); i++)
		{	
			SegSuperPixelComposeFeature& curfeat = *innerSegs[i];
			rate += curfeat.leftInnerArea * curfeat.importWeight;

			// use all other segments to fill this one in order of their similarity to this one
			curfeat.composition_cost = 0;
			for (size_t pi = 0; pi < curfeat.pairs.size(); pi++)
			{
				SegSuperPixelComposeFeature& feat = prim_sp_feats[curfeat.pairs[pi].id].compose_feat;
				if (feat.leftOuterArea <= 0) continue;

				float filled_area = 0;

				if (curfeat.leftInnerArea <= feat.leftOuterArea)	//enough
				{
					filled_area = curfeat.leftInnerArea;
					curfeat.leftInnerArea = 0;
					feat.leftOuterArea -= curfeat.leftInnerArea;
				}
				else
				{
					filled_area = feat.leftOuterArea;
					curfeat.leftInnerArea -= feat.leftOuterArea;
					feat.leftOuterArea = 0;
				}

				curfeat.composition_cost += curfeat.pairs[pi].saliency * filled_area * curfeat.importWeight;

				if (curfeat.leftInnerArea <= 0)	//finish
					break;
			}

			if (curfeat.leftInnerArea > 0)	// fill with maximum distance 1
			{				
				curfeat.composition_cost += 0.7 * curfeat.leftInnerArea;
				curfeat.leftInnerArea = 0;
			}

			winscore += curfeat.composition_cost;	// MODIFIED
		}

		const float winarea_inv = 1.0f / sp_feat.area;
		rate *= winarea_inv;
		return winscore * winarea_inv * rate;

	}

}

