
#include "stdafx.h"
#include "ObjSegmentProposal.h"

namespace objectproposal
{
	ObjSegmentProposal::ObjSegmentProposal(void)
	{
		verbose = false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool ObjSegmentProposal::GetCandidatesFromIterativeSeg(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps) {

		iter_segmentor.merge_method = MERGE_ITER;
		iter_segmentor.seg_size_bound_ = Point2f(0.005f, 0.5f);
		iter_segmentor.Init(cimg, dmap);
		iter_segmentor.Run2();
		sps = iter_segmentor.sps;

		return true;
	}

	/**
		1) compute saliency map
		2) select candidate fg/bg regions
		3) run watershed or graph cut to get regions
		4) iteratively compute saliency and repeat
		5) post processing
	*/
	bool ObjSegmentProposal::GetCandidatesFromSaliency(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps)
	{
		visualsearch::processors::attention::SaliencyComputer sal_comp;
		ShapeAnalyzer shape;
		sps.clear();

		Feature3D feat3d;
		Mat pts3d;
		if (!dmap.empty()) {
			feat3d.ComputeKinect3DMap(dmap, pts3d, true);
			pts3d.convertTo(pts3d, CV_8U, 255);
			if (verbose) {
				imshow("3d map", pts3d);
				waitKey(0);
			}
		}

		vector<visualsearch::processors::attention::SaliencyType> sal_types;
		//sal_types.push_back(visualsearch::processors::attention::SAL_HC);
		sal_types.push_back(visualsearch::processors::attention::SAL_GEO);

		for (size_t i = 0; i < sal_types.size(); i++)
		{
			// compute saliency map
			Mat salmap;
			sal_comp.ComputeSaliencyMap(cimg, sal_types[i], salmap);

			visualsearch::processors::segmentation::ImageSegmentor segmentor;
			segmentor.m_dThresholdK = 20.f;
			segmentor.seg_type_ = visualsearch::processors::segmentation::OVER_SEG_GRAPH;
			cout << "seg num " << segmentor.DoSegmentation(cimg) << endl;
			VisualObjects& raw_sps = segmentor.superPixels;
			// compute superpixel saliency map
			Mat sp_sal_map = Mat::zeros(cimg.rows, cimg.cols, CV_32F);
			for (auto& p : raw_sps) {
				float mean_sp_saliency = mean(salmap, p.visual_data.mask).val[0];
				p.visual_data.scores.push_back(mean_sp_saliency);
				sp_sal_map.setTo(mean_sp_saliency, p.visual_data.mask);
			}
			normalize(sp_sal_map, sp_sal_map, 1, 0, NORM_MINMAX);
			Mat oimg;
			ImgVisualizer::DrawFloatImg("sal", sp_sal_map, oimg);
			//imwrite("sal_old.jpg", cimg);
			//imwrite("sal.jpg", oimg);
			//waitKey(10);
			if (verbose) {
				ImgVisualizer::DrawFloatImg("sal", sp_sal_map);
				waitKey(0);
			}

			// compute superpixel distance
			Mat adj_mat;
			segmentor.ComputeAdjacencyMat(raw_sps, adj_mat);
			ColorDescriptors color_desc;
			ColorFeatParams cparams;
			cparams.feat_type = ColorFeatType::MEAN;
			cparams.mean_params.color_space = FeatColorSpace::RGB;
			color_desc.Init(cparams);
			for (auto& sp : raw_sps) {
				color_desc.Compute(cimg, sp.visual_data.custom_feats["color"], sp.visual_data.mask);
				sp.visual_data.custom_feats["color"] /= 255;
			}
			VSGraph g(raw_sps.size());
			for (size_t i = 0; i < raw_sps.size(); i++) {
				g[i][i] = 0;
				for (size_t j = i + 1; j < raw_sps.size(); j++) {
					if (adj_mat.at<int>(i, j) > 0)
						g[i][j] = g[j][i] = norm(raw_sps[i].visual_data.custom_feats["color"], raw_sps[j].visual_data.custom_feats["color"], NORM_L2);
				}
			}
			ObjectSegmentor obj_seg;

			// thresholding to get candidates
			// method 1: interval
			//vector<float> ths{ 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f };
			//for (size_t j = 0; j < ths.size(); j++) {
			//	// 0.4 is the minimum foreground threshold
			//	for (float th = 0.2; th < 1; th += 0.1f) {
			//		Mat cur_th_map;
			//		inRange(sp_sal_map, th, MIN(1, th+ths[j]), cur_th_map);
			//		cur_th_map.convertTo(cur_th_map, CV_8U);
			//		if (verbose) {
			//			imshow("th map", cur_th_map);
			//			waitKey(10);
			//		}
			//		vector<VisualObject> objs;
			//		shape.ExtractConnectedComponents(cur_th_map, objs);
			//		/*for (auto& obj : objs) {
			//			obj.visual_data.scores.push_back(mean(sp_sal_map, obj.visual_data.mask).val[0]);
			//			}*/
			//		sps.insert(sps.end(), objs.begin(), objs.end());
			//		cout << "sp num: " << sps.size() << endl;
			//	}
			//}
			// method 2: binary
			vector<float> fg_ths{ 0.9f, 0.8f, 0.7f, 0.6f};
			vector<float> bg_ths{ 0.1f, 0.2f };
			for (size_t j = 0; j < fg_ths.size(); j++) {
				for (size_t k = 0; k < bg_ths.size(); k++) {
					// classify sp
					vector<int> fg_sps, bg_sps;
					Mat bg_mask = Mat::zeros(cimg.rows, cimg.cols, CV_8U);
					for (size_t kk = 0; kk < raw_sps.size(); kk++) {
						if (raw_sps[kk].visual_data.scores[0] >= fg_ths[j]) {					
							fg_sps.push_back(kk);
							//fg_mask.setTo(255, raw_sps[kk].visual_data.mask);
						}
						if (raw_sps[kk].visual_data.scores[0] <= bg_ths[k]) {
							bg_sps.push_back(kk);
							bg_mask.setTo(255, raw_sps[kk].visual_data.mask);
						}
					}

					// try every combination
					for (auto fi : fg_sps) 
					{
						imshow("fg mask", raw_sps[fi].visual_data.mask * 255);
						imshow("bg mask", bg_mask);
						// run grab cut
						Mat obj_mask = Mat::zeros(cimg.rows, cimg.cols, CV_8U);
						obj_mask.setTo(GC_PR_BGD);
						obj_mask.setTo(GC_FGD, raw_sps[fi].visual_data.mask);
						obj_mask.setTo(GC_BGD, bg_mask);

						obj_seg.RunGrabCut(cimg, obj_mask, Rect(), false, true);
						//waitKey(0);

						// get components
						vector<VisualObject> objs;
					    shape.ExtractConnectedComponents(obj_mask, objs);
						sps.insert(sps.end(), objs.begin(), objs.end());

						// compute geodesic map
						/*Mat geo_dist;
						vector<ImageMaskSign> sp_signs(raw_sps.size(), ImageMaskSign::IMG_UNKNOWN);
						sp_signs[fi] = ImageMaskSign::IMG_FG;
						sp_signs[bi] = ImageMaskSign::IMG_BG;
						obj_seg.ComputeGeodesicObjectMap(g, sp_signs, geo_dist);
						Mat geo_obj_map = Mat::zeros(cimg.rows, cimg.cols, CV_32F);
						for (size_t kk = 0; kk < raw_sps.size(); kk++) {
						geo_obj_map.setTo(geo_dist.at<float>(kk), raw_sps[kk].visual_data.mask);
						}
						ImgVisualizer::DrawFloatImg("geo map", geo_obj_map);
						waitKey(0);*/
					}
				}
			}
		}

		cout << "total sp num: " << sps.size() << endl;
		// clean candidates
		for (auto& sp : sps) {
			seg_proc.ExtractSegmentBasicFeatures(sp);
		}
		SegmentProcessor::CleanSPs(sps, 0.01f, 0.2f, 0.2f);
		cout << "cleaned sp: " << sps.size() << endl;

		return true;
	}

	//bool ObjSegmentProposal::GetCandidatesFromSegment3D(const Mat& cimg, const Mat& dmap, vector<VisualObject>& sps) {
	//	// compute feature maps
	//	visualsearch::features::Feature3D feat3d;
	//	Mat pts_3d;
	//	feat3d.ComputeKinect3DMap(dmap, pts_3d, false);
	//	Mat color_bmap, color_float;
	//	cvtColor(cimg, color_float, CV_BGR2Lab);
	//	color_float.convertTo(color_float, CV_32F, 1.f/255);
	//	feat3d.ComputeBoundaryMap(color_float, Mat(), Mat(), features::BMAP_COLOR, color_bmap);
	//	Mat pts_bmap;
	//	feat3d.ComputeBoundaryMap(Mat(), pts_3d, Mat(), features::BMAP_3DPTS, pts_bmap);
	//	Mat normal_bmap, normal_map;
	//	feat3d.ComputeNormalMap(pts_3d, normal_map);
	//	feat3d.ComputeBoundaryMap(Mat(), Mat(), normal_map, features::BMAP_NORMAL, normal_bmap);
	//	
	//	//imshow("3d", pts_3d);
	//	ImgVisualizer::DrawNormals("normal", normal_map, Mat());
	//	ImgVisualizer::DrawFloatImg("color_bmap", color_bmap);
	//	ImgVisualizer::DrawFloatImg("pts3d_bmap", pts_bmap);
	//	ImgVisualizer::DrawFloatImg("normal_bmap", normal_bmap);
	//	//imwrite("bmap.png", pts_bmap);
	//	waitKey(10);

	//	// do segmentation
	//	vector<float> pts3d_bvals;
	//	pts3d_bvals.reserve(pts_bmap.rows*pts_bmap.cols);
	//	vector<float> normal_bvals;
	//	normal_bvals.reserve(pts3d_bvals.size());
	//	vector<float> color_bvals;
	//	color_bvals.reserve(pts3d_bvals.size());
	//	for(int r=0; r<pts_bmap.rows; r++) for(int c=0; c<pts_bmap.cols; c++) {
	//		pts3d_bvals.push_back(pts_bmap.at<float>(r,c));
	//		normal_bvals.push_back(normal_bmap.at<float>(r,c));
	//		color_bvals.push_back(color_bmap.at<float>(r,c));
	//	}
	//	sort(color_bvals.begin(), color_bvals.end());
	//	sort(pts3d_bvals.begin(), pts3d_bvals.end());
	//	sort(normal_bvals.begin(), normal_bvals.end());
	//	float color_ths[6] = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35};
	//	float pts_ths[6] = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35};
	//	float normal_ths[6] = {0.3, 0.35, 0.4, 0.5, 0.55, 0.6};
	//	sps.clear();
	//	Segmentor3D seg3d;
	//	for(int k=0; k<3; k++) {
	//		cout<<k<<endl;
	//		for(int i=0; i<6; i++) {
	//			int th_pos = 0;
	//			if(k==0) {
	//				th_pos = color_bvals.size() - (int)(color_bvals.size()*color_ths[i]);
	//				seg3d.DIST_TH = color_bvals[th_pos];
	//			}
	//			if(k==1) {
	//				th_pos = pts3d_bvals.size() - (int)(pts3d_bvals.size()*pts_ths[i]);
	//				seg3d.DIST_TH = pts3d_bvals[th_pos];
	//			}
	//			if(k==2) {
	//				th_pos = normal_bvals.size() - (int)(normal_bvals.size()*normal_ths[i]);
	//				seg3d.DIST_TH = normal_bvals[th_pos];
	//			}
	//			cout<<"th: "<<seg3d.DIST_TH<<endl;

	//			double start_t = getTickCount();
	//			vector<SuperPixel> res_objs;
	//			if(k==0)
	//				seg3d.RunRegionGrowing(color_bmap, res_objs);
	//			if(k==1)
	//				seg3d.RunRegionGrowing(pts_bmap, res_objs);
	//			if(k==2)
	//				seg3d.RunRegionGrowing(normal_bmap, res_objs);
	//			cout<<"Time cost: "<<(double)(getTickCount()-start_t) / getTickFrequency()<<"s."<<endl;

	//			sps.insert(sps.end(), res_objs.begin(), res_objs.end());
	//			waitKey(10);
	//		}
	//	}
	//	

	//	return true;
	//}

	//////////////////////////////////////////////////////////////////////////

	bool ObjSegmentProposal::Run(const Mat& cimg, const Mat& dmap, int topK, vector<VisualObject>& res)
	{
		// get candidates
		vector<VisualObject> res_sps;
		//GetCandidatesFromSegment3D(cimg, dmap, res_sps);
		//GetCandidatesFromIterativeSeg(cimg, dmap, res_sps);
		GetCandidatesFromSaliency(cimg, dmap, res_sps);
		// extract basic features
		SegmentProcessor seg_proc;
		//for(auto& sp : res_sps) seg_proc.ExtractBasicSegmentFeatures(sp, cimg, dmap);
		cout<<"object candidates: "<<res_sps.size()<<endl;

		// rank
		double start_t = getTickCount();
		cout<<endl<<"Ranking segments..."<<endl;
		vector<int> rank_ids;
		seg_ranker.RankSegments(cimg, dmap, res_sps, visualsearch::processors::attention::SEG_RANK_SALIENCY, rank_ids);
		cout << "ranking time: " << (getTickCount() - start_t) / getTickFrequency() << "s" << endl;

		// post-processing
		cout<<"Filter overlap segments..."<<endl;
		int valid_num = 0;
		vector<bool> valid_seg(rank_ids.size(), true);
		//for (size_t i=0; i<rank_ids.size(); i++) {
		//	if( !valid_seg[i] ) continue;
		//	// too big segments
		//	if( res_sps[rank_ids[i]].area > cimg.rows*cimg.cols*0.8f || res_sps[rank_ids[i]].area < cimg.rows*cimg.cols*0.01f ) {
		//		valid_seg[i] = false;
		//		valid_num++;
		//		continue;
		//	}
		//	// overlapping
		//	/*for(size_t j=i+1; j<rank_ids.size(); j++) {
		//	if( valid_seg[j] && 
		//	(float)countNonZero(
		//	res_sps[rank_ids[i]].mask & res_sps[rank_ids[j]].mask) / 
		//	countNonZero(res_sps[rank_ids[i]].mask | res_sps[rank_ids[j]].mask) > 0.5) {
		//	valid_seg[j] = false;
		//	valid_num++;
		//	}
		//	}*/
		//}
		valid_num = valid_seg.size() - valid_num;
		cout << "final proposal num: " << valid_num << endl << endl;

		res.clear();
		if(topK == -1) res.reserve(valid_seg.size());
		else res.reserve(topK);
		for (int i=0; i<valid_seg.size(); i++) {
			if(valid_seg[i]) res.push_back(res_sps[rank_ids[i]]);
			if(res.size() == topK) break;
		}

		return true;
	}

	bool ObjSegmentProposal::VisProposals(const Mat& cimg, const vector<VisualObject>& res)
	{
		char str[30];
		for (size_t i=0; i<res.size(); i++)
		{
			sprintf_s(str, "%d proposal", i);
			Mat dispimg(cimg.rows, cimg.cols, CV_8UC3);
			dispimg.setTo(Vec3b(0,0,0));
			cimg.copyTo(dispimg, res[i].visual_data.mask);
			rectangle(dispimg, res[i].visual_data.bbox, CV_RGB(255, 0, 0));
			imshow(str, dispimg);
			waitKey(0);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	void ObjSegmentProposal::ComputePRCurves(const vector<VisualObject>& ranked_objs, const vector<Mat>& gt_masks, float cover_th, 
		vector<Point2f>& pr_vals, vector<Point3f>& best_overlap, bool seg_or_win /* = true */) {
		SegmentProcessor seg_proc;
		vector<VisualObject> gt_objs(gt_masks.size());
		for (size_t i=0; i<gt_masks.size(); i++) {
			gt_masks[i].copyTo(gt_objs[i].visual_data.mask);
			seg_proc.ExtractSegmentBasicFeatures(gt_objs[i]);
		}

		// x: gt obj; y: proposal obj; z: jaccard index
		best_overlap.clear();
		best_overlap.resize(gt_masks.size());
		for(auto& pt : best_overlap) { pt = Point3f(0,0,0); }
		vector<bool> detect_gt(gt_masks.size(), false);
		vector<Point2f> tmp_vals(ranked_objs.size());
		for(size_t i=0; i<ranked_objs.size(); i++) {

			tmp_vals[i] = (i==0? Point2f(0,0): tmp_vals[i-1]);
			// try each gt object
			for(size_t j=0; j<gt_objs.size(); j++) {
				float cover_rate = 0;
				if(seg_or_win) {
					cover_rate = countNonZero(ranked_objs[i].visual_data.mask & gt_objs[j].visual_data.mask)*1.f / 
						countNonZero(ranked_objs[i].visual_data.mask | gt_objs[j].visual_data.mask);
				}
				else
					cover_rate = (ranked_objs[i].visual_data.bbox & gt_objs[j].visual_data.bbox).area()*1.f /
					(ranked_objs[i].visual_data.bbox | gt_objs[j].visual_data.bbox).area();

				if(cover_rate > best_overlap[j].z) {
					best_overlap[j].z = cover_rate;
					best_overlap[j].x = j;
					best_overlap[j].y = i;
				}
				if(cover_rate >= cover_th) {
					tmp_vals[i].y++;
					if( !detect_gt[j] ) { detect_gt[j] = true; tmp_vals[i].x++; }
				}
			}
		}

		/*for(size_t i=0; i<best_proposals.size(); i++) {
		imshow("gt mask", gt_masks[i]*255);
		imshow("proposal", ranked_objs[best_proposals[i]].mask*255);
		cout<<best_overlap[i]<<endl;
		waitKey(0);
		}*/

		pr_vals.resize(tmp_vals.size());
		// normalize
		for(size_t i=0; i<tmp_vals.size(); i++) {
			pr_vals[i].x = tmp_vals[i].x / gt_masks.size();
			pr_vals[i].y = tmp_vals[i].y / (i+1);
		}

	}
}


