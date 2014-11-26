//////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "ObjectRanker.h"
using namespace visualsearch;
using namespace visualsearch::features;
using namespace visualsearch::processors::attention;
using namespace visualsearch::processors::segmentation;
using namespace visualsearch::learners::trees;


class SegTreeNode {

public:
	int node_id;
	bool isLeaf;
	vector<int> samp_ids;

	NodeStatisticsHist trainingDataStatistics;
	Mat feat_weights;
	Mat obj_clf;
};

struct SegTreeParams {

	int max_depth;
	int feat_num;

};

//////////////////////////////////////////////////////////////////////////

class SegmentationTree {

public:
	SegmentationTree() { 
		feat_types.resize(7);
		feat_types[0] = BMAP_COLOR;
		feat_types[1] = BMAP_3DPTS;
		feat_types[2] = BMAP_NORMAL;
		feat_types[3] = BMAP_COLOR + BMAP_3DPTS;
		feat_types[4] = BMAP_COLOR + BMAP_NORMAL;
		feat_types[5] = BMAP_3DPTS + BMAP_NORMAL;
		feat_types[6] = BMAP_COLOR + BMAP_3DPTS + BMAP_NORMAL;

		node_feat_num = 20;
	}

	bool Train(const vector<VisualObject>& train_objs);

	bool Test(const VisualObject& obj);

	vector<SuperPixel> sps;

private:
	bool GetSegmentType(const Mat& mask, const Mat& obj_mask) const;

	bool TrainNode(int node_id, vector<VisualObject>& train_objs, const vector<int>& samp_ids);

	processors::segmentation::ImageSegmentor img_segmentor;
	processors::attention::SaliencyComputer sal_computer;

	vector<SegTreeNode> nodes;
	vector<int> feat_types;
	int node_feat_num;
};



class ObjectSearcher
{
public:
	ObjectSearcher(void);

	bool SplitSearch(const Mat& cimg, const Mat& dmap);

	bool SplitSearchIteration(int sp_id, const Mat& cimg, const Mat& pts3d_map, const Mat& normal_map);

	vector<SuperPixel> sps;

private:
	SaliencyComputer sal_comp;

	float ComputeSalmapEntropy(const Mat& sal_map, const Mat& mask, int bin_num);
};



