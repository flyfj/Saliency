#include "Segmentor3D.h"
#include "..\Shared\Learners\LibLinear\blas\blas.h"


Segmentor3D::Segmentor3D(void)
{
	DIST_TH = 0.02f;
}


bool Segmentor3D::Run(const vector<vector<FeatPoint>>& super_img) {

	int imgw = super_img[0].size();
	int imgh = super_img.size();
	labels.create(imgh, imgw, CV_32S);
	labels.setTo(-1);
	labels.at<int>(0, 0) = 0;
	int label_cnt = 1;
	map<int, set<int>> equal_labels;	// save equivalent labels
	set<float> dists;
	// two-pass labeling algorithm
	// only need to look top-left ones
	Point offsets[3] = {Point(-1, 0), Point(-1, -1), Point(0, -1)};
	for (int r=0; r<imgh; r++) {
		for(int c=0; c<imgw; c++) {
			if(labels.at<int>(r,c) == -1) {
				// look at neighborhood
				bool found_sim = false;
				for(int i=0; i<3; i++) {
					int newx = MIN(MAX(0, c+offsets[i].x), imgw-1);
					int newy = MIN(MAX(0, r+offsets[i].y), imgh-1);
					Point refpt(newx, newy);
					// don't compare with self
					if(refpt.x == c && refpt.y == r) continue;

					int ref_label = labels.at<int>(refpt);
					int& cur_label = labels.at<int>(r, c);
					float dist = FeatPoint::ComputeDist(super_img[r][c], super_img[refpt.y][refpt.x]);
					dists.insert(dist);
					if( dist < DIST_TH ) {
						if( cur_label == -1 ) {
							cur_label = ref_label;
						}
						else {
							if(ref_label != cur_label) { 
								equal_labels[MIN(cur_label, ref_label)].insert(MAX(cur_label, ref_label));
							}
						}
						found_sim = true;
					}
				}
				// no similar neighbors, give new label
				if(!found_sim) labels.at<int>(r, c) = label_cnt++;
			}
		}
	}

	// aggregate equivalent labels
	int new_label_cnt = 0;
	vector<int> new_labels(label_cnt, -1);
	for(auto pi=equal_labels.begin(); pi!=equal_labels.end(); pi++) {
		cout<<pi->first<<endl;
		if(new_labels[pi->first] == -1) 
			new_labels[pi->first] = new_label_cnt++;
		for(auto val : pi->second) new_labels[val] = new_labels[pi->first];
	}
	for(int& lval : new_labels) {
		if(lval == -1) lval = new_label_cnt++;
	}
	cout<<"total labels: "<<new_label_cnt;

	// update label map
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		labels.at<int>(r, c) = new_labels[labels.at<int>(r, c)];
	}

	// draw
	vector<Vec3b> seg_colors(new_label_cnt);
	for(size_t i=0; i<seg_colors.size(); i++) 
		seg_colors[i] = Vec3b((uchar)rand()%255, (uchar)rand()%255, (uchar)rand()%255);
	Mat seg_img(labels.rows, labels.cols, CV_8UC3);
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		seg_img.at<Vec3b>(r, c) = seg_colors[labels.at<int>(r,c)];
	}
	imshow("segimg", seg_img);
	cv::waitKey(0);

	return true;
}

bool Segmentor3D::RunRegionGrowing(const Mat& pts3d_bmap, vector<SuperPixel>& res_sps) {

	Mat bmap_th;
	threshold(pts3d_bmap, bmap_th, DIST_TH, 255, CV_THRESH_BINARY_INV);
	imshow("bmap_th", bmap_th);
	waitKey(10);

	res_sps.clear();
	res_sps.reserve(100);
	labels.create(pts3d_bmap.rows, pts3d_bmap.cols, CV_32S);
	labels.setTo(-1);
	int label_cnt = 0;
	for(int r=0; r<pts3d_bmap.rows; r++) for(int c=0; c<pts3d_bmap.cols; c++) {
		if(labels.at<int>(r, c) == -1 && bmap_th.at<float>(r,c) > 0) {
			Mat cur_mask(pts3d_bmap.rows+2, pts3d_bmap.cols+2, CV_8U);
			cur_mask.setTo(0);
			Mat bmap = bmap_th.clone();
			Rect valid_roi = Rect(1, 1, cur_mask.cols-2, cur_mask.rows-2);
			int flags = 8 + (255 << 8) + CV_FLOODFILL_MASK_ONLY;
			Rect ccomp;
			int area = floodFill(bmap, cur_mask, Point(c, r), 255, &ccomp, Scalar(DIST_TH, DIST_TH, DIST_TH), Scalar(DIST_TH, DIST_TH, DIST_TH), flags);
			SuperPixel new_sp;
			cur_mask(valid_roi).copyTo(new_sp.mask);
			res_sps.push_back(new_sp);
			labels.setTo(label_cnt++, new_sp.mask);
		}
	}
	cout<<"proposal num: "<<label_cnt<<endl;

	// draw
	cv::RNG rng_gen(NULL);
	vector<Vec3b> seg_colors(label_cnt);
	for(size_t i=0; i<seg_colors.size(); i++)
		seg_colors[i] = Vec3b((uchar)rng_gen.uniform(0, 255), (uchar)rng_gen.uniform(0, 255), (uchar)rng_gen.uniform(0, 255));
	Mat seg_img(labels.rows, labels.cols, CV_8UC3);
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		int label = labels.at<int>(r,c);
		seg_img.at<Vec3b>(r, c) = (label == -1? Vec3b(0,0,0):seg_colors[label]);
	}
	imshow("segimg", seg_img);
	//cv::waitKey(0);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool Segmentor3D::TrainBoundaryDetector(DatasetName db_name) {
	string temp_dir = "E:\\Results\\objectness\\";	// save intermediate results

	DataManagerInterface* db_man = NULL;
	map<string, vector<Mat>> objmasks;
	if( (db_name & DB_NYU2_RGBD) != 0 )
		db_man = new NYUDepth2DataMan();
	if( (db_name & DB_SALIENCY_RGBD) != 0 )
		db_man = new RGBDECCV14();

	FileInfos imgfiles, dmapfiles;
	db_man->GetImageList(imgfiles);
	random_shuffle(imgfiles.begin(), imgfiles.end());
	imgfiles.erase(imgfiles.begin()+15, imgfiles.end());
	db_man->GetDepthmapList(imgfiles, dmapfiles);
	db_man->LoadGTMasks(imgfiles, objmasks);

	// processors
	processors::segmentation::SegmentProcessor seg_processor;
	features::Feature3D feat3d;

	// collect training samples
	std::cout<<"Generating training samples..."<<endl;
	Mat possamps, negsamps;
	int samp_num_per_img = 200;
	int kernel_size = 5;
	for(size_t i=0; i<imgfiles.size(); i++) {
		// input data
		Mat cimg = imread(imgfiles[i].filepath);
		Size newsz;
		tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
		resize(cimg, cimg, newsz);
		cimg.convertTo(cimg, CV_32F, 1.f/255);
		Mat dmap;
		db_man->LoadDepthData(dmapfiles[i].filepath, dmap);
		resize(dmap, dmap, newsz);

		// compute feature map
		Mat pts3d_map, normal_map, color_bmap, pts3d_bmap, normal_bmap;
		feat3d.ComputeKinect3DMap(dmap, pts3d_map);
		feat3d.ComputeNormalMap(pts3d_map, normal_map);

		// vis
		imshow("color", cimg*255);
		ImgVisualizer::DrawFloatImg("depth", dmap);
		imshow("3d", pts3d_map*255);
		imshow("normal", normal_map*255);
		waitKey(10);
		//feat3d.ComputeBoundaryMap(pts3d_map, features::BMAP_3DPTS, pts3d_bmap);
		//feat3d.ComputeBoundaryMap(cimg, features::BMAP_COLOR, color_bmap);
		//feat3d.ComputeBoundaryMap(normal_map, features::BMAP_NORMAL, normal_bmap);

		// randomly sample subset of boundary points as positive ones, others as negative ones
		// mark all contour points first, then sample
		vector<Mat>& gt_masks = objmasks[imgfiles[i].filename];
		Mat contour_mask(cimg.rows, cimg.cols, CV_8U);
		contour_mask.setTo(0);
		for (size_t j=0; j<gt_masks.size(); j++) {
			SuperPixel cur_gt_obj;
			cur_gt_obj.mask = gt_masks[j];
			resize(cur_gt_obj.mask, cur_gt_obj.mask, newsz);
			seg_processor.ExtractBasicSegmentFeatures(cur_gt_obj, cimg, dmap);
			
			//vector<SuperPixel> sps;
			//sps.push_back(cur_gt_obj);
			//ImgVisualizer::DrawShapes(cimg, sps);
			//waitKey(0);
			for(auto pt : cur_gt_obj.original_contour) {
				contour_mask.at<uchar>(pt) = 255;
			}
		}

		vector<Point> pos_pts, neg_pts;
		for(int r=0; r<cimg.rows; r++) for(int c=0; c<cimg.cols; c++) {
			if(contour_mask.at<uchar>(r, c) == 255) pos_pts.push_back(Point(c, r));
			else neg_pts.push_back(Point(c, r));
		}
		random_shuffle(pos_pts.begin(), pos_pts.end());
		random_shuffle(neg_pts.begin(), neg_pts.end());

		int max_samp_sz = MIN(samp_num_per_img, MIN(pos_pts.size(), neg_pts.size()));
		for(int k=0; k<max_samp_sz; k++) {
			Point cur_pts[2];
			cur_pts[0] = pos_pts[k];
			cur_pts[1] = neg_pts[k];
			for(int kk=0; kk<2; kk++) {
				bool pt_valid = true;
				if(cur_pts[kk].x < kernel_size/2 || cur_pts[kk].y < kernel_size/2 || 
					cur_pts[kk].x > cimg.cols-1-kernel_size/2 || cur_pts[kk].y > cimg.rows-1-kernel_size/2)
					continue;

				Rect cur_roi(cur_pts[kk].x-kernel_size/2, cur_pts[kk].y-kernel_size/2, kernel_size, kernel_size);
				vector<Mat> feat_maps(3);
				cimg(cur_roi).copyTo(feat_maps[0]);
				pts3d_map(cur_roi).copyTo(feat_maps[1]);
				normal_map(cur_roi).copyTo(feat_maps[2]);
				// construct total feature
				Mat cur_feat(1, kernel_size*kernel_size*9, CV_32F);
				int cnt = 0;
				for(int id=0; id<3; id++) {
					for(int ch=0; ch<3; ch++) {
						for(int r=0; r<kernel_size; r++) for(int c=0; c<kernel_size; c++) {
							cur_feat.at<float>(cnt++) = feat_maps[id].at<Vec3f>(r, c).val[ch];
						}
					}
				}
				if(kk == 0) possamps.push_back(cur_feat);
				else negsamps.push_back(cur_feat);
			}

		}

		cout<<"Sampled image "<<i<<endl;
	}

	// divide into train and test data
	Mat rank_train_data, rank_test_data, rank_train_label, rank_test_label;
	for (int r=0; r<possamps.rows; r++) {
		if(r<possamps.rows*0.8) {
			rank_train_data.push_back(possamps.row(r));
			rank_train_label.push_back(1);
		}
		else {
			rank_test_data.push_back(possamps.row(r));
			rank_test_label.push_back(1);
		}
	}
	for (int r=0; r<negsamps.rows; r++) {
		if(r<negsamps.rows*0.8) {
			rank_train_data.push_back(negsamps.row(r));
			rank_train_label.push_back(0);
		}
		else {
			rank_test_data.push_back(negsamps.row(r));
			rank_test_label.push_back(0);
		}
	}

	delete db_man;
	db_man = NULL;
	cout<<"Rank training data ready."<<endl;

	cout<<"Start to train..."<<endl;
	ofstream out("bound_forest.dat");
	learners::trees::DecisionTree dtree;
	learners::trees::DTreeTrainingParams tparams;
	tparams.feat_type = learners::trees::DTREE_FEAT_CONV;
	tparams.feature_num = 400;
	tparams.th_num = 100;
	tparams.min_samp_num = 50;
	learners::trees::RandomForest rforest;
	learners::trees::RForestTrainingParams rfparams;
	rfparams.num_trees = 4;
	rfparams.split_disjoint = false;
	rfparams.tree_params = tparams;
	rforest.Train(rank_train_data, rank_train_label, rfparams);
	rforest.Save(out);
	//dtree.TrainTree(rank_train_data, rank_train_label, tparams);
	//dtree.Save("bound_tree.dat");

	rforest.EvaluateRandomForest(rank_test_data, rank_test_label, 2);
	//dtree.EvaluateDecisionTree(rank_test_data, rank_test_label, 2);

	return true;
}

bool Segmentor3D::RunBoundaryDetection(const Mat& cimg, const Mat& dmap, Mat& bmap) {
	// load classifier
	learners::trees::DecisionTree dtree;
	learners::trees::DTreeTrainingParams tparams;
	tparams.feat_type = learners::trees::DTREE_FEAT_CONV;
	tparams.feature_num = 400;
	tparams.th_num = 100;
	tparams.min_samp_num = 50;
	dtree.defaultParams = tparams;
	learners::trees::RandomForest rforest;
	learners::trees::RForestTrainingParams rfparams;
	rfparams.num_trees = 4;
	rfparams.split_disjoint = false;
	rfparams.tree_params = tparams;
	rforest.defaultParams = rfparams;
	ifstream in("bound_forest.dat");
	if( !rforest.Load(in) )
		return false;
	
	// compute feature maps
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
	features::Feature3D feat3d;
	vector<Mat> feat_maps(3);
	cimg.convertTo(feat_maps[0], CV_32F, 1.f/255);
	resize(feat_maps[0], feat_maps[0], newsz);
	Mat new_dmap;
	resize(dmap, new_dmap, newsz);
	feat3d.ComputeKinect3DMap(new_dmap, feat_maps[1]);
	feat3d.ComputeNormalMap(feat_maps[1], feat_maps[2]);

	bmap.create(newsz.height, newsz.width, CV_32F);
	bmap.setTo(0);
	//Mat extend_cmap;
	//copyMakeBorder(color_map, extend_cmap, 2, 2, 2, 2, BORDER_REFLECT);
	for(int r=2; r<newsz.height-2; r++) for(int c=2; c<newsz.width-2; c++) {
		Mat cur_feat(1, 5*5*9, CV_32F);
		int cnt = 0;
		int minx = c-2;
		int miny = r-2;
		for(int i=0; i<3; i++) {
			for(int ch=0; ch<3; ch++) {
				for(int rr=miny; rr<miny+5; rr++) for(int cc=minx; cc<minx+5; cc++) {
					cur_feat.at<float>(cnt++) = feat_maps[i].at<Vec3f>(rr, cc).val[ch];
				}
			}
		}

		vector<double> allscores;
		int pred_cls = rforest.ForestPredict(cur_feat, allscores);
		bmap.at<float>(r, c) = (float)allscores[1];
	}
	
	double minv, maxv;
	minMaxLoc(bmap, &minv, &maxv);
	cout<<minv<<" "<<maxv<<endl;
	//medianBlur(bmap, bmap, 3);
	ImgVisualizer::DrawFloatImg("bmap", bmap);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////

// use up too much memory
void Segmentor3D::BFS(int x, int y, int label, const vector<vector<FeatPoint>>& super_img) {
	int imgw = super_img[0].size();
	int imgh = super_img.size();
	Point offsets[4] = {Point(-1, 0), Point(0, -1), Point(1, 0), Point(0, 1)};

	queue<Point> to_visit_pts;
	to_visit_pts.push(Point(x, y));
	while(!to_visit_pts.empty()) {	
		Point cur_pt = to_visit_pts.front();
		to_visit_pts.pop();
		labels.at<int>(cur_pt) = label;
		bool found_new = false;
		for(int i=0; i<4; i++) {
			int newx = MIN(MAX(0, cur_pt.x+offsets[i].x), imgw-1);
			int newy = MIN(MAX(0, cur_pt.y+offsets[i].y), imgh-1);
			if(newx == cur_pt.x && newy == cur_pt.y) continue;
			if(labels.at<int>(newy, newx) == -1) {
				float dist = FeatPoint::ComputeDist(super_img[cur_pt.y][cur_pt.x], super_img[newy][newx]);
				if(dist < DIST_TH) {
					to_visit_pts.push(Point(newx, newy));
					found_new = true;
				}
			}
		}

		if(!found_new)
			cout<<to_visit_pts.size()<<endl;
	}
}

bool Segmentor3D::RunBFS(const vector<vector<FeatPoint>>& super_img) {
	int imgw = super_img[0].size();
	int imgh = super_img.size();
	labels.create(imgh, imgw, CV_32S);
	labels.setTo(-1);
	int label_cnt = 0;

	for (int r=0; r<imgh; r++) for(int c=0; c<imgw; c++) {
		if(labels.at<int>(r, c) == -1) BFS(c, r, label_cnt++, super_img);
	}

	// draw
	vector<Vec3b> seg_colors(label_cnt);
	for(size_t i=0; i<seg_colors.size(); i++) 
		seg_colors[i] = Vec3b((uchar)rand()%255, (uchar)rand()%255, (uchar)rand()%255);
	Mat seg_img(labels.rows, labels.cols, CV_8UC3);
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		seg_img.at<Vec3b>(r, c) = seg_colors[labels.at<int>(r,c)];
	}
	imshow("segimg", seg_img);
	cv::waitKey(0);

	return true;
}