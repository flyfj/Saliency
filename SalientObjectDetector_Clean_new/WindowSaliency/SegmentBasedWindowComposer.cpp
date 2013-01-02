#include "SegmentBasedWindowComposer.h"

#include <algorithm>

bool SegSuperPixelFeature::use4Neighbor = false;

//////////////////////////////////////////////////////////////////////////
// compute spatial distance
float Point2RectDistance(int x, int y, const Rect& box)
{
	if (box.Contains(x,y)) return 0;

	if(x<box.X)
	{
		if(y<box.Y)
			return l2_dist(x, y, box.X, box.Y);
		else if(y<box.GetBottom())
			return (box.X-x);
		else
			return l2_dist(x, y, box.X, box.GetBottom());
	}
	else if(x<box.GetRight())
	{
		if(y<box.Y)
			return (box.Y-y);
		else if(y>box.GetBottom())
			return (y-box.GetBottom());
		else
			return 0;
	}
	else
	{
		if(y<box.Y)
			return l2_dist(x, y, box.GetRight(), box.Y);
		else if(y<box.GetBottom())
			return (x-box.GetRight());
		else
			return l2_dist(x, y, box.GetRight(), box.GetBottom());
	}
}

float HausdorffDist(Rect a, Rect b)
{
	// compute approximate Hausdorff distance
	// a->b
	float mindist1 = 999999999.f;
	Rect box1 = a;
	Rect box2 = b;

	vector<Point> points(4);
	points[0] = Point(box1.X, box1.Y);
	points[1] = Point(box1.X, box1.GetBottom());
	points[2] = Point(box1.GetRight(), box1.Y);
	points[3] = Point(box1.GetRight(), box1.GetBottom());

	for(size_t id = 0; id < 4; id++)
	{
		float dist = Point2RectDistance(points[id].X, points[id].Y, box2);
		if(dist < mindist1)
			mindist1 = dist;
	}

	// b->a
	float mindist2 = 999999999.9f;
	points[0] = Point(box2.X, box2.Y);
	points[1] = Point(box2.X, box2.GetBottom());
	points[2] = Point(box2.GetRight(), box2.Y);
	points[3] = Point(box2.GetRight(), box2.GetBottom());

	for(size_t id = 0; id < 4; id++)
	{
		float dist = Point2RectDistance(points[id].X, points[id].Y, box1);
		if(dist < mindist2)
			mindist2 = dist;
	}

	return max(mindist1, mindist2);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// a wrapper class to use SegmentBasedWindowComposer as an EHSW func
class EHSWFunc
{
	SegmentBasedWindowComposer& composer;
public:
	typedef float fun_result_type;
	int numOfUpdateBinCalls;

	EHSWFunc(SegmentBasedWindowComposer& c)	: composer(c) {}

	unsigned int HistLength() const { return composer.sp_comp_features.size(); }
	bool IsBinUseful(const unsigned int& b) const	{	return true;	}

	template<class HistType>
	inline float Evaluate(HistType& hist)
	{
		assert(false);
		return 0;
	}

	inline float Evaluate(HistArray& hist, unsigned int win_x, unsigned int win_y, unsigned int win_width, unsigned int win_height)
	{
		// TODO: following 2 lines cause frequent memory allocation for innerSegs
		// could be avoided by own implementation
		composer.innerSegs.clear();
		composer.innerSegs.reserve(HistLength());

		PointF winCenter(win_x+win_width/2, win_y+win_height/2);
		for(int n = 0; n < hist.size(); n++)
		{
			SegSuperPixelComposeFeature& spfeat = composer.sp_comp_features[n];
			
			int segment_area = hist[n];
			if (segment_area > 0)
			{
				spfeat.InitFillArea(segment_area);

				// prepare inside segments
				if (spfeat.leftInnerArea > 0)
				{
					spfeat.dist_to_win = l2_dist(spfeat.centroid, winCenter);
					composer.innerSegs.push_back(&spfeat);
				}
			}
			else
				spfeat.InitFillArea();
		}

		//return composer.innerSegs.size();  // useless, just for debug purpose

		float score = composer.compose_greedy(Rect(win_x, win_y, win_width, win_height));
		return score;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////
SegmentBasedWindowComposer::SegmentBasedWindowComposer() : m_nImgWidth(0), m_nImgHeight(0)
	,seg_index_map(NULL)
	,m_fMaxCost(0.7f)
{
}

SegmentBasedWindowComposer::~SegmentBasedWindowComposer()
{
	Clear();
}

void SegmentBasedWindowComposer::Clear()
{
	sp_comp_features.clear();
	innerSegs.clear();	// clear pointers
}

bool SegmentBasedWindowComposer::Init(const ImageUIntSimple& seg_map, const vector<SegSuperPixelFeature>& sp_features, const ImageFloatSimple* bg_weight_map)
{
	//Clear();	no need to clear again: call once in detector::init
	
	// init data
	m_nImgWidth = seg_map.Width();
	m_nImgHeight = seg_map.Height();
	seg_index_map = &seg_map;
	compose_cost_map.Create(m_nImgWidth, m_nImgHeight);
	m_fMaxCost = 0.7f;

#ifdef RECORD_AUXILIARY_INFO
	compose_cost_map.Create(m_nImgWidth, m_nImgHeight);
#endif


	//////////////////////////////////////////////////////////////////////////
	// compute composition features for each superpixel
	//////////////////////////////////////////////////////////////////////////
	sp_comp_features.resize(sp_features.size());

	for (size_t i=0; i<sp_comp_features.size(); i++)
	{
		// set common data
		sp_comp_features[i].box = sp_features[i].box;
		sp_comp_features[i].centroid = sp_features[i].centroid;

		// boundary area extension
		sp_comp_features[i].extendedArea = sp_features[i].area;
		if(sp_features[i].bnd_pixels > 0)
			sp_comp_features[i].extendedArea *= (1 + 2*(float)sp_features[i].bnd_pixels / sp_features[i].perimeter);
		
		// create mask integral
		sp_comp_features[i].CreateAreaIntegral(seg_map, i);
	}

	// compute background weight for each superpixel
	if(bg_weight_map != NULL)
	{
		for(size_t i=0; i<sp_features.size(); i++)
		{
			Rect box = sp_features[i].box;
			sp_comp_features[i].importWeight = 0;
			for(int y=box.Y; y<box.GetBottom(); y++)
			{
				for(int x=box.X; x<box.GetRight(); x++)
				{
					if(seg_map.Pixel(x, y) == i)
					{
						sp_comp_features[i].importWeight += bg_weight_map->Pixel(x,y);
					}
				}
			}
			sp_comp_features[i].importWeight /= sp_features[i].area;
			
			// if the cost is above some threshold, set the pixel weight to 1 to avoid over-focusing locally
			if(sp_comp_features[i].importWeight > 0.3f)	
				sp_comp_features[i].importWeight = 1.0f;
		}
	}

	// compute pair-wise composition cost
	float maxAppdist = 0, maxSpadist = 0;
	for (int curIdx = 0; curIdx < sp_features.size(); curIdx++)
	{	
		// allocate space
		sp_comp_features[curIdx].pairs.reserve(sp_comp_features.size());

		// temp object
		TPair pair;
		for (int nextIdx = 0; nextIdx < sp_features.size(); nextIdx++)
		{			
			pair.id = nextIdx;

			// appearance distance
			pair.appdist = SegSuperPixelFeature::FeatureIntersectionDistance(sp_features[curIdx], sp_features[nextIdx]);

			if (pair.appdist > maxAppdist)
				maxAppdist = pair.appdist;

			// spatial distance
			pair.spadist = HausdorffDist(sp_features[curIdx].box, sp_features[nextIdx].box);

			if (pair.spadist > maxSpadist)
				maxSpadist = pair.spadist;

			// add to list
			sp_comp_features[curIdx].pairs.push_back(pair);
		}
	}
	// normalize
	for (int n = 0; n < sp_comp_features.size(); n++)
	{
		for(size_t i = 0; i < sp_comp_features[n].pairs.size(); i++)
		{			
			sp_comp_features[n].pairs[i].appdist /= maxAppdist;
			sp_comp_features[n].pairs[i].spadist /= maxSpadist;
		}
	}
	// sort all lists
	for (int n = 0; n < sp_comp_features.size(); n++)
	{
		SegSuperPixelComposeFeature& curfeat = sp_comp_features[n];
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

inline float SegmentBasedWindowComposer::compose_greedy(const Rect& win)
{
	// compose order
	sort(innerSegs.begin(), innerSegs.end(), SegSuperPixelComposeFeature::comp_by_dist);
	
	float rate = 0;	// compute weight ratio to penalize background window
	float winscore = 0;

#ifdef RECORD_AUXILIARY_INFO
	compose_cost_map.FillPixels(0);
#endif

	for (size_t i = 0; i < innerSegs.size(); i++)
	{		
		SegSuperPixelComposeFeature& curfeat = *innerSegs[i];
		rate += curfeat.leftInnerArea * curfeat.importWeight;

#ifdef RECORD_AUXILIARY_INFO
		curfeat.composers.clear();
#endif

		// use all other segments to fill this one in order of their similarity to this one
		curfeat.composition_cost = 0;
		for (size_t pi = 0; pi < curfeat.pairs.size(); pi++)
		{
			SegSuperPixelComposeFeature& feat = sp_comp_features[curfeat.pairs[pi].id];
			// don't use itself, this condition is redundant and implied in the one below
			//if (curfeat.pairs[pi].id == innerSegs[i].id) continue;
			if (feat.leftOuterArea <= 0) continue;

			float fillarea = 0;

			if (curfeat.leftInnerArea <= feat.leftOuterArea)	//enough
			{
				fillarea = curfeat.leftInnerArea;
				feat.leftOuterArea -= curfeat.leftInnerArea;
				curfeat.leftInnerArea = 0;
			}
			else
			{
				fillarea = feat.leftOuterArea;
				curfeat.leftInnerArea -= feat.leftOuterArea;
				feat.leftOuterArea = 0;
			}

			curfeat.composition_cost += curfeat.pairs[pi].saliency * fillarea * curfeat.importWeight;

#ifdef RECORD_AUXILIARY_INFO
			// add composer
			curfeat.composers.push_back(curfeat.pairs[pi]);
#endif

			if (curfeat.leftInnerArea <= 0)	//finish
				break;
		}

		if (curfeat.leftInnerArea > 0)	// fill with maximum distance 1
		{				
			curfeat.composition_cost += m_fMaxCost * curfeat.leftInnerArea;
			curfeat.leftInnerArea = 0;
		}
		
		winscore += curfeat.composition_cost;	// MODIFIED

#ifdef RECORD_AUXILIARY_INFO
		// set composition cost map
		for(size_t i=0; i<curfeat.mask.size(); i++)
		{
			Point loc(i%curfeat.box.Width, i/curfeat.box.Width);
			if(curfeat.mask[i] > 0)
				compose_cost_map.Pixel(loc.X, loc.Y) = curfeat.composition_cost;
		}
#endif
	}

	const float winarea_inv = 1.0f / (win.Width*win.Height);
	rate *= winarea_inv;
	return winscore * winarea_inv * rate;

}

float SegmentBasedWindowComposer::Compose(const Rect& win)
{	
	innerSegs.clear();
	innerSegs.reserve( sp_comp_features.size() );

	// init all segments' inside and outside area with respect to the window
	// and init prepare all inside segments
	PointF winCenter(win.X+win.Width/2, win.Y+win.Height/2);

	for(size_t i=0; i<sp_comp_features.size(); i++)
	{
		// initialize area
		SegSuperPixelComposeFeature& curfeat = sp_comp_features[i];
		float innerArea = curfeat.AreaIn(win);

		// reset composition cost
		curfeat.composition_cost = 0;

		if (innerArea > 0)
		{
			curfeat.InitFillArea(innerArea);
			// prepare inside segments
			if (curfeat.leftInnerArea > 0)
			{
				curfeat.dist_to_win = l2_dist(sp_comp_features[i].centroid, winCenter);
				innerSegs.push_back(&curfeat);
			}
		}
		else
			curfeat.InitFillArea();
	}

	return compose_greedy(win);

}

void SegmentBasedWindowComposer::ComposeAll(const int win_width, const int win_height, const bool use_ehsw)
{		
	compose_cost_map.FillPixels(0);

	if (use_ehsw)
	{		
		ImageResultWriter<ImageFloatSimple, OptimumFinderWriter<float, less<float>>> writer(compose_cost_map, win_width/2, win_height/2);

		EHSWFunc func(*this);
		// 1. dense sparse
		typedef incr_histogram_calculator<scanline_sliding_window_calculator<ImageUIntSimple, AccuHistBase<EHSWFunc, HistArray>, HistSparse, false, true> > DenseSparseFullCalculator;
		DenseSparseFullCalculator calculator(win_width, win_height, m_nImgWidth, m_nImgHeight, *seg_index_map, win_height <= win_width, func);

		// 2. sparse sparse, need to initialize segment's area
		//typedef incr_histogram_calculator<scanline_sliding_window_calculator<CImage<PixelClusterIndexType>, AccuHistBase<EHSWFunc, HistSparse>, HistSparse, false, true> > SparseSparseFullCalculator;
		//SparseSparseFullCalculator calculator(win.Width, win.Height, pRes->m_nWidth, pRes->m_nHeight, pSegRes->index_map.m_index, win.Height <= win.Width, func);
		//for(int n = 0; n < pRes->sp_feature.size(); n++)	pRes->sp_feature[n].InitFillArea();		

		incr_scanline_calculate(calculator, 0, 0, writer);
	}
	else // brute force computation
	{	
		ScoredRect win(Rect(0, 0, win_width, win_height));
		for(win.Y = 0; win.Y < m_nImgHeight - win.Height+1; win.Y++)
		{
			for(win.X = 0; win.X < m_nImgWidth - win.Width+1; win.X++)
			{				
				win.score = Compose(win);
				compose_cost_map.Pixel(win.X+win.Width/2, win.Y+win.Height/2) = win.score;
			}
		}
	}
}