#pragma once


#include "common.h"

#include <vector>
using namespace std;

namespace Saliency
{
	// make every structure clear; pass only necessary information to algorithm
	// 1. redefine data structure, only include truly necessary data members, also differentiate those only for algorithm analysis (using RECORD_AUXILIARY_INFO)
	// 2. always initialize variables and make them as private as possible
	// 3. make meaningful variable names and add comments to variables and function blocks

	// if RECORD_AUXILIARY_INFO is defined, those intermediate results in algorithm are recorded for analysis
	// should NOT be defined in the final, and the code still runs correctly

	//#define RECORD_AUXILIARY_INFO


	inline float l2_dist(float x1, float y1, float x2, float y2)
	{
		return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
	}

	inline float l2_dist(const Point2f& p1, const Point2f& p2)
	{
		return l2_dist(p1.x, p1.y, p2.x, p2.y);
	}

	struct ScoredRect : public Rect
	{
		ScoredRect() : score(0){}
		ScoredRect(const Rect& r) : Rect(r), score(0){}
		ScoredRect(const Rect& r, float s) : Rect(r), score(s){}
		float score;

		static bool comp_by_score(const ScoredRect& a, const ScoredRect& b)
		{
			return a.score < b.score;
		}
	};

	// information of another segment
	struct TPair
	{
		TPair(): id(-1), appdist(0), spadist(0), saliency(0)	{}
	
		int id;
		float appdist;	// appearance distance
		float spadist;	// spatial distance
		float saliency;	// pair composition cost

		static bool comp_by_saliency(const TPair& a, const TPair& b) 
		{
			return a.saliency < b.saliency;	
		}
	};


	// superpixel dynamic features only used for composition
	struct SegSuperPixelComposeFeature
	{
		SegSuperPixelComposeFeature() : composition_cost(0.f)
			,extendedArea(0), leftInnerArea(0), leftOuterArea(0), currentArea(0)
			,dist_to_win(0), importWeight(1), centroid(0,0)
		{}

		//void Init(const ImageFloatSimple& saliencyMatrix);	

		vector<TPair> pairs;	// list for all other superpixels
	
		float extendedArea;	// area after boundary extension
		float leftInnerArea, leftOuterArea;	// inside and outside areas with respect to a window after self-composing
		float composition_cost;	// cost for composing current superpixel
		float currentArea;	// area need to be filled currently
		float importWeight;	// importance weight: may affected by relative position, background likelihood...	
		Point2f centroid;
		Rect box;	// bounding box

	#ifdef RECORD_AUXILIARY_INFO
		vector<TPair> composers;	 // segments composing current sp
	#endif


		//////////////////////////////////////////////////////////////////////////
		inline void InitFillArea(int area_in_win)
		{		
			assert(area_in_win > 0);
			leftInnerArea = area_in_win;
			leftOuterArea = extendedArea - leftInnerArea;

			if (leftInnerArea > leftOuterArea)
			{
				leftInnerArea -=  leftOuterArea;
				leftOuterArea = 0;
			}
			else
			{
				leftInnerArea = 0;
				leftOuterArea -= leftInnerArea;
			}

			currentArea = leftInnerArea;
		}

		inline void InitFillArea()
		{
			leftInnerArea = 0;
			leftOuterArea = extendedArea;

			currentArea = 0;
		}

		float dist_to_win;	// distance between centroid to window center

		static bool comp_by_dist(const SegSuperPixelComposeFeature* a, const SegSuperPixelComposeFeature* b)	{	return a->dist_to_win > b->dist_to_win;	}

		static bool comp_by_cost(const SegSuperPixelComposeFeature* a, const SegSuperPixelComposeFeature* b)	{	return a->composition_cost < b->composition_cost;	}

		// x: id, y: area
		static bool comp_by_area(const Point a, const Point b) { return a.Y > b.Y; }

	};

	// it holds the key data structure and algorithm for window based composition algorithm
	class SegmentBasedWindowComposer
	{
	public:
		vector<SegSuperPixelComposeFeature> sp_comp_features;	// superpixel features

		SegmentBasedWindowComposer();
		~SegmentBasedWindowComposer();

		friend class EHSWFunc;

		// input parameters: a rgb image, a segmentation index image
		// 1. compute background weight for each segment
		// 2. compute pair-wise composition cost
		// return whether initialization is done correctly, usually the memory allocation status
		bool Init(const Mat& seg_map, const vector<SegSuperPixelFeature>& sp_features, const ImageFloatSimple* bg_weight_map = NULL); 

		// compute composition cost of a single window
		float Compose(const Rect& win);
	
		// compute composition costs of all windows	in compose_cost_map		
		void ComposeAll(const int win_width, const int win_height, const bool use_ehsw = true);

	protected:
		void Clear();  // release internal buffers

		int m_nImgWidth, m_nImgHeight;

		// cost map of a sliding window, of dimension (img_width, img_height)
		// but only dimension (img_width-win_width+1, img_height-win_height+1) is filled
		ImageFloatSimple compose_cost_map;

	private:
		const ImageUIntSimple* seg_index_map;	// input segmentation map, set once in Init()

		vector<SegSuperPixelComposeFeature*> innerSegs;	// segments inside current window
		inline float compose_greedy(const Rect& win);

		float m_fMaxCost; // cost upper bound

	#ifdef RECORD_AUXILIARY_INFO
		ImageFloatSimple comp_cost_map;		 // window segment composition cost image
	#endif
	
	};
}

