//////////////////////////////////////////////////////////////////////////
//	implementation
//////////////////////////////////////////////////////////////////////////

#include "CompositionCost.h"

//////////////////////////////////////////////////////////////////////////
inline float l2_dist(float x1, float y1, float x2, float y2)
{
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

inline float Point2RectDistance(int x, int y, const Rect& box)
{
	if (box.contains(Point(x,y))) return 0;

	if(x<box.x)
	{
		if(y<box.y)
			return l2_dist(x, y, box.x, box.y);
		else if(y<box.y+box.height-1)
			return (box.x-x);
		else
			return l2_dist(x, y, box.x, box.y+box.height-1);
	}
	else if(x<box.x+box.width-1)
	{
		if(y<box.y)
			return (box.y-y);
		else if(y>box.y+box.height-1)
			return (y-(box.y+box.height-1));
		else
			return 0;
	}
	else
	{
		if(y<box.y)
			return l2_dist(x, y, box.x+box.width-1, box.y);
		else if(y<box.y+box.height-1)
			return (x-(box.x+box.width-1));
		else
			return l2_dist(x, y, box.x+box.width-1, box.y+box.height-1);
	}
}

float HausdorffDist(const Superpixel& a, const Superpixel& b)
{
	// compute approximate Hausdorff distance
	// a->b
	float mindist1 = INFINITE;
	Rect box1 = a.bbox;
	Rect box2 = b.bbox;

	vector<Point> points(4);
	points[0] = Point(box1.x, box1.y);
	points[1] = Point(box1.x, box1.y+box1.height-1);
	points[2] = Point(box1.x+box1.width-1, box1.y);
	points[3] = Point(box1.x+box1.width-1, box1.y+box1.height-1);

	for(size_t id = 0; id < 4; id++)
	{
		float dist = Point2RectDistance(points[id].x, points[id].y, box2);
		if(dist < mindist1)
			mindist1 = dist;
	}

	// b->a
	float mindist2 = INFINITE;
	points[0] = Point(box2.x, box2.y);
	points[1] = Point(box2.x, box2.y+box2.height-1);
	points[2] = Point(box2.x+box2.width-1, box2.y);
	points[3] = Point(box2.x+box2.width-1, box2.y+box2.height-1);

	for(size_t id = 0; id < 4; id++)
	{
		float dist = Point2RectDistance(points[id].x, points[id].y, box1);
		if(dist < mindist2)
			mindist2 = dist;
	}

	return max(mindist1, mindist2);
}

//////////////////////////////////////////////////////////////////////////
void SegSuperPixelFeature::Init(const Superpixel& sp, const Mat& lab_img)
{
	extendedArea = sp.area;
	// boundary expansion
	
	if(sp.ifBoundarySeg)
		extendedArea *= 1.5;

	// compute lab color histogram
	const int quantBins[3] = {4, 8, 8}; //used in paper

	feat.resize(quantBins[0]+quantBins[1]+quantBins[2], 0);	//L_A_B

	for (int y=sp.bbox.y; y<sp.bbox.y+sp.bbox.height; y++)
	{
		for(int x=sp.bbox.x; x<sp.bbox.x+sp.bbox.width; x++)
		{
			if(sp.sp_mask.at<uchar>(y, x) == 0)
				continue;

			float l = (float)lab_img.at<Vec3b>(y, x)[0];
			float a = (float)lab_img.at<Vec3b>(y, x)[1];
			float b = (float)lab_img.at<Vec3b>(y, x)[2];
			int lbin = (int)(l/(255.f/quantBins[0]));
			lbin = ( lbin > quantBins[0]-1? quantBins[0]-1: lbin );
			int abin = (int)(a/(255.f/quantBins[1]));
			abin = ( abin > quantBins[1]-1? quantBins[1]-1: abin );
			int bbin = (int)(b/(255.f/quantBins[2]));
			bbin = ( bbin > quantBins[2]-1? quantBins[2]-1: bbin );

			feat[lbin]++;
			feat[quantBins[0]+abin]++;
			feat[quantBins[0]+quantBins[1]+bbin]++;
		}
	}

	//do normalization
	for(size_t i=0; i<feat.size(); i++)
		feat[i] /= (3*sp.area);
}
//////////////////////////////////////////////////////////////////////////
void CompositionCost::ClearAll()
{
	m_img.release();
	sp_features.clear();
}

void CompositionCost::Init(const Mat& img)
{
	// clear
	ClearAll();

	// init
	m_img.create(img.size(), img.depth());
	img.copyTo(m_img);
	m_fMaxCost = 0.5f;

	// do segmentation
	m_segmentor.DoSegmentation(img);

	/*
		do initialization
		compute pairwise superpixel appearance/spatial distance
	*/
	Mat labImg(img.size(), CV_8UC3);
	cvtColor(img, labImg, CV_BGR2Lab);	// scale to 0~255

	// select half superpixels with top areas
	vector<Point> bigsegs;
	// compute superpixel features
	vector<Superpixel>& superpixels = m_segmentor.superpixels;
	sp_features.resize(superpixels.size());
	for (size_t i=0; i<superpixels.size(); i++)
	{
		sp_features[i].Init(superpixels[i], labImg);
		sp_features[i].id = i;
		Point p;
		p.x = i;	// id
		p.y = superpixels[i].area;	// area
		bigsegs.push_back(p);
	}

	sort(bigsegs.begin(), bigsegs.end(), SegSuperPixelFeature::comp_by_area);
	bigsegs.resize(bigsegs.size()/2);

	// compute distance between two superpixels	
	float maxAppdist = 0, maxSpadist = 0;
	for (int curIdx = 0; curIdx < sp_features.size(); curIdx++)
	{		
		TPair pair;
		for (int nextIdx = 0; nextIdx < sp_features.size(); nextIdx++)
		{			
			pair.id = nextIdx;

			// appearance distance
			pair.appdist = SegSuperPixelFeature::FeatureIntersectionDistance(sp_features[curIdx], sp_features[nextIdx]);
			if (pair.appdist > maxAppdist)
				maxAppdist = pair.appdist;

			// spatial distance
			pair.spadist = HausdorffDist(superpixels[curIdx], superpixels[nextIdx]);

			if (pair.spadist > maxSpadist)
				maxSpadist = pair.spadist;

			// add to list
			sp_features[curIdx].pairs.push_back(pair);
		}
	}

	// normalize distance and output
	// get normalization coefficient
	//set<float> appdists;
	//for(size_t i=0; i<bigsegs.size(); i++)
	//{
	//	int seg1 = bigsegs[i].x;
	//	for(size_t j=0; j<bigsegs.size(); j++)
	//	{
	//		int seg2 = bigsegs[j].x;
	//		appdists.insert(sp_features[seg1].pairs[seg2].appdist);
	//	}
	//}
	//// use 90% value as threshold
	//int id = (int)((appdists.size()-1)*0.9);
	//set<float>::iterator pi = appdists.begin();
	//for(;id>0;pi++, id--);
	//	maxAppdist = *(pi);

	for (int n = 0; n < sp_features.size(); n++)
	{
		for(size_t i = 0; i < sp_features[n].pairs.size(); i++)
		{			
			sp_features[n].pairs[i].appdist /= maxAppdist;
			if(sp_features[n].pairs[i].appdist > 1)
				sp_features[n].pairs[i].appdist = 1;

			sp_features[n].pairs[i].spadist /= maxSpadist;
		}
	}

	// sort all lists
	for (int n = 0; n < sp_features.size(); n++)
	{
		SegSuperPixelFeature& curfeat = sp_features[n];
		// compute distance(similarity) with other superpixels
		for(size_t pi = 0; pi < curfeat.pairs.size(); pi++)
		{			
			float weight = curfeat.pairs[pi].spadist;
			if (weight > 1)
				weight = 1;

			// set saliency value for each superpixel
			curfeat.pairs[pi].saliency = (1-weight)*curfeat.pairs[pi].appdist + weight*m_fMaxCost;
		}

		// sort by saliency
		sort(curfeat.pairs.begin(), curfeat.pairs.end(), TPair::comp_by_saliency);
	}
}

float CompositionCost::ComputeCompositionCost(const Mat& mask)
{

	Mat mask_show(mask.size(), CV_8U);
	mask_show.setTo(0);

#pragma region initialize areas

	// compute inner and outer area of each segment
	const vector<Superpixel>& superpixels = m_segmentor.superpixels;
	vector<SegSuperPixelFeature*> innerSegs;
	innerSegs.clear();
	innerSegs.reserve(superpixels.size());

	// init all superpixels' inside and outside area with respect to the window
	// and init prepare all inside superpixels
	// compute 1st spatial moment as segment center
	Moments segMoments = moments(mask, true);
	Point segCenter;
	segCenter.x = (int)(segMoments.m10 / segMoments.m00);
	segCenter.y = (int)(segMoments.m01 / segMoments.m00);
	// draw centroid
	//circle(mask_show, segCenter, 1, CV_RGB(255,255,255));
	/*imshow("cur mask", mask);
	waitKey(10);*/

	// init area
	for(size_t i=0; i<superpixels.size(); i++)
	{
		const Superpixel& sp = superpixels[i];
		Mat AndMask(sp.sp_mask.size(), CV_8U);
		assert(sp.sp_mask.size() == mask.size());
		bitwise_and(mask, sp.sp_mask, AndMask);

		int innerArea = countNonZero(AndMask);

		// initialize area
		SegSuperPixelFeature& curfeat = sp_features[i];

		// reset composition cost
		curfeat.composition_cost = 0;

		if (innerArea > 0)
		{
			curfeat.InitFillArea(innerArea);
			// prepare inside superpixels
			if (curfeat.leftInnerArea > 0)
			{
				curfeat.dist_to_win = l2_dist(sp.centroid.x, sp.centroid.y, (float)segCenter.x, (float)segCenter.y);
				innerSegs.push_back(&curfeat);
			}
		}
		else
			curfeat.InitFillArea();
	}

#pragma endregion initialize areas

#pragma region compose

	int MaskArea = countNonZero(mask);
	// loop inner superpixels to compose
	sort(innerSegs.begin(), innerSegs.end(), SegSuperPixelFeature::comp_by_dist);

	float costTH = 0;
	float winscore = 0;
	for (size_t i = 0; i < innerSegs.size(); i++)
	{
		SegSuperPixelFeature& curfeat = *innerSegs[i];

		curfeat.composers.clear();

		// use all other superpixels to fill this one in order of their similarity to this one
		curfeat.composition_cost = 0;
		for (size_t pi = 0; pi < curfeat.pairs.size(); pi++)
		{
			SegSuperPixelFeature& feat = sp_features[curfeat.pairs[pi].id];
			// don't use itself, this condition is redundant and implied in the one below
			//if (curfeat.pairs[pi].id == innerSegs[i].id) continue;
			if (feat.leftOuterArea <= 0) continue;

			float fillarea = 0;
			if (curfeat.leftInnerArea <= feat.leftOuterArea)	//enough
			{
				fillarea = curfeat.leftInnerArea;
				curfeat.leftInnerArea = 0;
				feat.leftOuterArea -= curfeat.leftInnerArea;
			}
			else
			{
				fillarea = feat.leftOuterArea;
				curfeat.leftInnerArea -= feat.leftOuterArea;
				feat.leftOuterArea = 0;
			}

			curfeat.composition_cost += curfeat.pairs[pi].saliency * fillarea;

			// add composer
			curfeat.composers.push_back(curfeat.pairs[pi]);

			if (curfeat.leftInnerArea <= 0)	//finish
				break;
		}

		if (curfeat.leftInnerArea > 0)	// fill with maximum distance 1
		{				
			curfeat.composition_cost += m_fMaxCost * curfeat.leftInnerArea;	// modified to constrain its upper bound
			curfeat.leftInnerArea = 0;
		}

		winscore += curfeat.composition_cost;	// MODIFIED
	}

	float winarea_inv = 1.0f / MaskArea;

	winscore *= winarea_inv;

#pragma endregion compose
	
	return winscore;

}