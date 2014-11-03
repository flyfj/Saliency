#include "ObjectSearcher.h"

//////////////////////////////////////////////////////////////////////////

bool SegmentationTree::TrainNode(int node_id, vector<VisualObject>& train_objs, const vector<int>& samp_ids) {

	double maxGain = -1;
	Mat best_weights;
	Mat best_clf;
	for(int k=0; k<node_feat_num; k++) {
		// generate random cut params
		RNG rng_gen(getTickCount());
		Mat mix_weights(1, 3, CV_32F);
		rng_gen.fill(mix_weights, RNG::UNIFORM, 0, 1);
		normalize(mix_weights, mix_weights, 1, 0, NORM_L1);

		Mat clf_weights(1, 3, CV_32F);
		rng_gen.fill(clf_weights, RNG::UNIFORM, 0, 1);

		// set up segmentor
		

		NodeStatisticsHist left_hist(2);
		NodeStatisticsHist right_hist(2);
		// process all samples
		for(size_t i=0; i<train_objs.size(); i++) {
			VisualObject& cur_obj = train_objs[i];
			// feature map
			Mat feat_map = mix_weights.at<float>(0)*cur_obj.visual_desc.extra_features["color_bmap"] + 
				mix_weights.at<float>(1)*cur_obj.visual_desc.extra_features["pts3d_bmap"] + 
				mix_weights.at<float>(2)*cur_obj.visual_desc.extra_features["normal_bmap"];
			feat_map.convertTo(feat_map, CV_8U, 255);
			cvtColor(feat_map, feat_map, CV_GRAY2BGR);

			// do segmentation
			img_segmentor.DoSegmentation(feat_map);

			// evaluate
			for(auto seg : img_segmentor.superPixels) {
				// extract feature


				int cur_label = GetSegmentType(seg.mask, cur_obj.visual_desc.extra_features["obj_mask"]);
				if( seg.namedFeats["obj_feat"].dot(clf_weights) < 0.5f )
					left_hist.AddSample(cur_label, 1);
				else
					right_hist.AddSample(cur_label, 1);
			}
		}

		// compute entropy
		double ig = NodeStatisticsHist::EvaluateSeparability(nodes[node_id].trainingDataStatistics,
			left_hist, right_hist, SEP_CRIT_IG);
		if(ig > maxGain) {
			best_weights = mix_weights;
			best_clf = clf_weights;
		}
	}

	// set current node


	// create new nodes
	

	return true;
}


bool SegmentationTree::GetSegmentType(const Mat& mask, const Mat& obj_mask) const {
	int intersect_area = countNonZero(mask & obj_mask);
	int obj_area = countNonZero(obj_mask);
	if(intersect_area*1.f/obj_area > 0.9)
		return true;
	else
		return false;
}


bool SegmentationTree::Train(const vector<VisualObject>& train_objs) {



	return true;
}



//////////////////////////////////////////////////////////////////////////

ObjectSearcher::ObjectSearcher(void)
{
}


bool ObjectSearcher::SplitSearchIteration(int sp_id, const Mat& cimg, const Mat& pts3d_map, const Mat& normal_map) {
	const Mat& cur_mask = sps[sp_id].mask;
	Mat invalid_mask = 1 - cur_mask;
	Mat cimg_sub, pts3d_sub, normal_sub;
	cimg_sub.create(cimg.rows, cimg.cols, CV_8UC3);
	cimg_sub.setTo(Vec3b(0,0,0));
	pts3d_sub = cimg_sub.clone();
	normal_sub = cimg_sub.clone();
	cimg.copyTo(cimg_sub, cur_mask);
	pts3d_map.copyTo(pts3d_sub, cur_mask);
	normal_map.copyTo(normal_map, cur_mask);

	// find the feature gives most confident saliency map
	Mat color_sal, pts3d_sal, normal_sal;
	sal_comp.ComputeSaliencyMap(cimg, SAL_HC, color_sal);
	ImgVisualizer::DrawFloatImg("color sal before", color_sal);
	color_sal.setTo(0, invalid_mask);
	normalize(color_sal, color_sal, 1, 0, NORM_MINMAX);
	ImgVisualizer::DrawFloatImg("color sal after", color_sal);

	sal_comp.ComputeSaliencyMap(pts3d_map, SAL_HC, pts3d_sal);
	ImgVisualizer::DrawFloatImg("pts sal before", pts3d_sal);
	pts3d_sal.setTo(0, invalid_mask);
	normalize(pts3d_sal, pts3d_sal, 1, 0, NORM_MINMAX);
	ImgVisualizer::DrawFloatImg("pts sal after", pts3d_sal);

	sal_comp.ComputeSaliencyMap(normal_map, SAL_HC, normal_sal);
	ImgVisualizer::DrawFloatImg("normal sal before", normal_sal);
	normal_sal.setTo(0, invalid_mask);
	normalize(normal_sal, normal_sal, 1, 0, NORM_MINMAX);
	ImgVisualizer::DrawFloatImg("normal sal after", normal_sal);

	float color_val = ComputeSalmapEntropy(color_sal, cur_mask, 20);
	float pts3d_val = ComputeSalmapEntropy(pts3d_sal, cur_mask, 20);
	float normal_val = ComputeSalmapEntropy(normal_sal, cur_mask, 20);
	cout<<"cimg sal: "<<color_val<<endl;
	cout<<"pts sal: "<<pts3d_val<<endl;
	cout<<"normal sal: "<<normal_val<<endl;
	waitKey(0);

	int optimal_map = 0;
	Mat optimal_sal;
	Mat optimal_feat;
	if(color_val < pts3d_val) {
		if(normal_val < color_val) {
			// normal
			optimal_map = BMAP_NORMAL;
			optimal_sal = normal_sal;
			optimal_feat = normal_map;
		}
		else {
			// color
			optimal_map = BMAP_COLOR;
			optimal_sal = color_sal;
			optimal_feat = cimg;
		}
	}
	else {
		if(pts3d_val < normal_val) {
			// pts
			optimal_map = BMAP_3DPTS;
			optimal_sal = pts3d_sal;
			optimal_feat = pts3d_map;
		}
		else {
			// normal
			optimal_map = BMAP_NORMAL;
			optimal_sal = normal_sal;
			optimal_feat = normal_map;
		}
	}

	ImageSegmentor segmentor;
	segmentor.m_dMinArea = 400;
	segmentor.m_dThresholdK = 3000;
	segmentor.seg_type_ = OVER_SEG_GRAPH;
	segmentor.DoSegmentation(optimal_feat);
	imshow("seg", segmentor.m_segImg);
	waitKey(0);
	return true;


	// diversify using different graph-cut parameters
	// parameter groups
	float fg_ths[4] = {0.9, 0.8, 0.7, 0.6};
	float bg_ths[4] = {0.1, 0.2, 0.3, 0.4};
	for(int i=0; i<4; i++) for(int j=0; j<4; j++) {
		Mat seg_mask(cur_mask.rows, cur_mask.cols, CV_8U);
		seg_mask.setTo(GC_PR_FGD);
		Mat fg_mask;
		compare(optimal_sal, fg_ths[i], fg_mask, CV_CMP_GE);
		Mat bg_mask;
		compare(optimal_sal, bg_ths[j], bg_mask, CV_CMP_LE);
		//seg_mask.setTo(GC_FGD, fg_mask);
		seg_mask.setTo(GC_BGD, bg_mask);
		ImgVisualizer::DrawFloatImg("seg mask", seg_mask);

		double start_t = getTickCount();
		Mat fgmodel, bgmodel;
		grabCut(optimal_feat, seg_mask, Rect(), bgmodel, fgmodel, 1, GC_INIT_WITH_MASK);
		cout<<"seg time: "<<(getTickCount()-start_t) / getTickFrequency()<<"s."<<endl;
		// get fg and bg mask
		fg_mask = seg_mask & 1;
		bg_mask = 1 - fg_mask;
		// show results
		fgmodel.release();
		bgmodel.release();
		cimg.copyTo(fgmodel, fg_mask);
		cimg.copyTo(bgmodel, bg_mask);
		imshow("fg", fgmodel);
		imshow("bg", bgmodel);
		waitKey(0);
	}


	return true;
}

bool ObjectSearcher::SplitSearch(const Mat& cimg, const Mat& dmap) {
	Mat lab_img;
	//cvtColor(cimg, lab_img, CV_BGR2Lab);
	Mat dmap_color;
	dmap.convertTo(dmap_color, CV_32F);

	features::Feature3D feat3d;
	Mat pts3d_map, normal_map;
	feat3d.ComputeKinect3DMap(dmap_color, pts3d_map, true);
	feat3d.ComputeNormalMap(pts3d_map, normal_map);

	// convert to color
	normalize(dmap, dmap_color, 1, 0, NORM_MINMAX, CV_32F);
	dmap_color.convertTo(dmap_color, CV_8U, 255);
	pts3d_map.convertTo(pts3d_map, CV_8U, 255);
	normal_map.convertTo(normal_map, CV_8U, 255);

	imshow("pts", pts3d_map);
	imshow("normal", normal_map);

	SuperPixel all_sp;
	all_sp.mask.create(cimg.rows, cimg.cols, CV_8U);
	all_sp.mask.setTo(1);
	sps.push_back(all_sp);

	SplitSearchIteration(0, cimg, pts3d_map, normal_map);
	
	return true;
}

float ObjectSearcher::ComputeSalmapEntropy(const Mat& sal_map, const Mat& mask, int bin_num) {
	float step = 1.f / bin_num;
	learners::trees::NodeStatisticsHist hist(bin_num);
	for (int r=0; r<sal_map.rows; r++) for(int c=0; c<sal_map.cols; c++) {
		if(mask.at<uchar>(r, c) == 0) continue;
		int bin_id = MIN(int(sal_map.at<float>(r,c)/step), bin_num-1);
		hist.AddSample(bin_id, 1);
	}

	return hist.Entropy();

}