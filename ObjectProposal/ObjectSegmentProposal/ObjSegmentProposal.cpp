#include "ObjSegmentProposal.h"

namespace objectproposal
{
	ObjSegmentProposal::ObjSegmentProposal(void)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	bool ObjSegmentProposal::GetCandidatesFromIterativeSeg(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps) {

		iter_segmentor.merge_feat_types = SP_COLOR;
		iter_segmentor.seg_size_bound_ = Point2f(0.01f, 0.75f);
		iter_segmentor.Init(cimg, dmap);
		iter_segmentor.verbose = false;
		iter_segmentor.Run();
		sps = iter_segmentor.sps;

		return true;
	}

	bool ObjSegmentProposal::GetCandidatesFromSegment3D(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps) {

		// compute feature maps
		features::Feature3D feat3d;
		Mat pts_3d;
		feat3d.ComputeKinect3DMap(dmap, pts_3d, false);
		Mat color_bmap, color_float;
		cvtColor(cimg, color_float, CV_BGR2Lab);
		color_float.convertTo(color_float, CV_32F, 1.f/255);
		feat3d.ComputeBoundaryMap(color_float, features::BMAP_COLOR, color_bmap);
		Mat pts_bmap;
		feat3d.ComputeBoundaryMap(pts_3d, features::BMAP_3DPTS, pts_bmap);
		Mat normal_bmap, normal_map;
		feat3d.ComputeNormalMap(pts_3d, normal_map);
		feat3d.ComputeBoundaryMap(normal_map, features::BMAP_NORMAL, normal_bmap);
		
		//imshow("3d", pts_3d);
		ImgVisualizer::DrawNormals("normal", normal_map);
		ImgVisualizer::DrawFloatImg("color_bmap", color_bmap);
		ImgVisualizer::DrawFloatImg("pts3d_bmap", pts_bmap);
		ImgVisualizer::DrawFloatImg("normal_bmap", normal_bmap);
		//imwrite("bmap.png", pts_bmap);
		waitKey(10);

		// do segmentation
		vector<float> pts3d_bvals;
		pts3d_bvals.reserve(pts_bmap.rows*pts_bmap.cols);
		vector<float> normal_bvals;
		normal_bvals.reserve(pts3d_bvals.size());
		vector<float> color_bvals;
		color_bvals.reserve(pts3d_bvals.size());
		for(int r=0; r<pts_bmap.rows; r++) for(int c=0; c<pts_bmap.cols; c++) {
			pts3d_bvals.push_back(pts_bmap.at<float>(r,c));
			normal_bvals.push_back(normal_bmap.at<float>(r,c));
			color_bvals.push_back(color_bmap.at<float>(r,c));
		}
		sort(color_bvals.begin(), color_bvals.end());
		sort(pts3d_bvals.begin(), pts3d_bvals.end());
		sort(normal_bvals.begin(), normal_bvals.end());
		float color_ths[6] = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35};
		float pts_ths[6] = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35};
		float normal_ths[6] = {0.3, 0.35, 0.4, 0.5, 0.55, 0.6};
		sps.clear();
		Segmentor3D seg3d;
		for(int k=0; k<3; k++) {
			cout<<k<<endl;
			for(int i=0; i<6; i++) {
				int th_pos = 0;
				if(k==0) {
					th_pos = color_bvals.size() - (int)(color_bvals.size()*color_ths[i]);
					seg3d.DIST_TH = color_bvals[th_pos];
				}
				if(k==1) {
					th_pos = pts3d_bvals.size() - (int)(pts3d_bvals.size()*pts_ths[i]);
					seg3d.DIST_TH = pts3d_bvals[th_pos];
				}
				if(k==2) {
					th_pos = normal_bvals.size() - (int)(normal_bvals.size()*normal_ths[i]);
					seg3d.DIST_TH = normal_bvals[th_pos];
				}
				cout<<"th: "<<seg3d.DIST_TH<<endl;

				double start_t = getTickCount();
				vector<SuperPixel> res_objs;
				if(k==0)
					seg3d.RunRegionGrowing(color_bmap, res_objs);
				if(k==1)
					seg3d.RunRegionGrowing(pts_bmap, res_objs);
				if(k==2)
					seg3d.RunRegionGrowing(normal_bmap, res_objs);
				cout<<"Time cost: "<<(double)(getTickCount()-start_t) / getTickFrequency()<<"s."<<endl;

				sps.insert(sps.end(), res_objs.begin(), res_objs.end());
				waitKey(10);
			}
		}
		

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool ObjSegmentProposal::Run(const Mat& cimg, const Mat& dmap, int topK, vector<SuperPixel>& res)
	{
		// get candidates
		vector<SuperPixel> res_sps;
		GetCandidatesFromSegment3D(cimg, dmap, res_sps);
		//GetCandidatesFromIterativeSeg(cimg, dmap, res_sps);
		// extract basic features
		SegmentProcessor seg_proc;
		for(auto& sp : res_sps) seg_proc.ExtractBasicSegmentFeatures(sp, cimg, dmap);
		cout<<"object candidates: "<<res_sps.size()<<endl;
		Mat shapes;
		ImgVisualizer::DrawShapes(cimg, res_sps, shapes, false);
		imshow("shapes", shapes);
		waitKey(10);

		// rank
		cout<<"Ranking segments..."<<endl;
		vector<int> rank_ids;
		seg_ranker.RankSegments(cimg, dmap, res_sps, visualsearch::processors::attention::SEG_RANK_SALIENCY, rank_ids);

		// post-processing
		// nms for segments
		cout<<"Filter overlap segments..."<<endl;
		int valid_num = 0;
		vector<bool> valid_seg(rank_ids.size(), true);
		for (size_t i=0; i<rank_ids.size(); i++) {
			if( !valid_seg[i] ) continue;
			// too big segments
			if( res_sps[rank_ids[i]].area > cimg.rows*cimg.cols*0.5f || res_sps[rank_ids[i]].area < cimg.rows*cimg.cols*0.01f ) {
				valid_seg[i] = false;
				valid_num++;
				continue;
			}
			// overlapping
			for(size_t j=i+1; j<rank_ids.size(); j++) {
				if( valid_seg[j] && 
					(float)countNonZero(
					res_sps[rank_ids[i]].mask & res_sps[rank_ids[j]].mask) / 
					countNonZero(res_sps[rank_ids[i]].mask | res_sps[rank_ids[j]].mask) > 0.5) {
					valid_seg[j] = false;
					valid_num++;
				}
			}
		}
		valid_num = valid_seg.size() - valid_num;
		cout<<"final proposal num: "<<valid_num<<endl;

		res.clear();
		res.reserve(topK);
		for (int i=0; i<valid_seg.size(); i++) {
			if(valid_seg[i]) res.push_back(res_sps[rank_ids[i]]);
			if(res.size() == topK) break;
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

	//////////////////////////////////////////////////////////////////////////

	void ObjSegmentProposal::ComputePRCurves(const vector<SuperPixel>& ranked_objs, const vector<Mat>& gt_masks, float cover_th, 
		vector<Point2f>& pr_vals, bool seg_or_win /* = true */) {
		SegmentProcessor seg_proc;
		vector<SuperPixel> gt_objs(gt_masks.size());
		for (size_t i=0; i<gt_masks.size(); i++) {
			gt_masks[i].copyTo(gt_objs[i].mask);
			seg_proc.ExtractBasicSegmentFeatures(gt_objs[i], Mat(), Mat());
		}

		vector<bool> detect_gt(gt_masks.size(), false);
		vector<Point2f> tmp_vals(ranked_objs.size());
		for(size_t i=0; i<ranked_objs.size(); i++) {
			tmp_vals[i] = (i==0? Point2f(0,0): tmp_vals[i-1]);
			// try each gt object
			for(size_t j=0; j<gt_objs.size(); j++) {
				float cover_rate = 0;
				if(seg_or_win) {
					cover_rate = countNonZero(ranked_objs[i].mask & gt_objs[j].mask)*1.f / countNonZero(ranked_objs[i].mask | gt_objs[j].mask);
				}
				else
					cover_rate = (ranked_objs[i].box & gt_objs[j].box).area()*1.f / (ranked_objs[i].box | gt_objs[j].box).area();

				if(cover_rate >= cover_th) {
					tmp_vals[i].y++;
					if( !detect_gt[j] ) { detect_gt[j] = true; tmp_vals[i].x++; }
				}
			}
		}

		pr_vals.resize(ranked_objs.size());
		// normalize
		for(size_t i=0; i<tmp_vals.size(); i++) {
			pr_vals[i].x = tmp_vals[i].x / gt_masks.size();
			pr_vals[i].y = tmp_vals[i].y / (i+1);
		}

	}
}


