#include "ObjPatchMatcher.h"


ObjPatchMatcher::ObjPatchMatcher(void)
{
	patch_size = Size(9, 9);
	use_depth = false;
	use_code = false;

	int sel_cls[] = {58, 7, 124, 24, 136, 157, 19,88, 3, 83, 5, 344, 238, 13, 80, 89, 408, 49, 66};
	valid_cls.resize(900, false);
	for(auto id : sel_cls) valid_cls[id] = true;

	uw_view_root = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\";
}

// normalized value concatenation
// gray patch has problem when contrast of object patch is flipped, gradient is more robust
bool ObjPatchMatcher::ComputePatchFeat(MatFeatureSet& patches, Mat& feat) {
	feat.release();
	feat.create(0, 0, CV_32F);
	if(patches.find("gray") != patches.end()) {
		Scalar mean_, std_;
		//normalize(patches["gray"], patches["gray"], 1, 0, NORM_MINMAX);
		meanStdDev(patches["gray"], mean_, std_);
		Mat patch_ = (patches["gray"]-0);///std_.val[0];
		if(feat.empty()) patch_.reshape(1,1).copyTo(feat);
		else hconcat(patch_.reshape(1, 1), feat, feat);
	}
	if(patches.find("gradient") != patches.end()) {
		normalize(patches["gradient"], patches["gradient"], 1, 0, NORM_MINMAX);
		Scalar mean_, std_;
		meanStdDev(patches["gradient"], mean_, std_);
		Mat patch_ = (patches["gradient"]-0);///std_.val[0];
		if(feat.empty()) patch_.reshape(1,1).copyTo(feat);
		else hconcat(patch_.reshape(1, 1), feat, feat);
	}
	if(patches.find("normal") != patches.end()) {
		Scalar mean_, std_;
		meanStdDev(patches["normal"], mean_, std_);
		//Mat patch_ = (patches["normal"]-mean_.val);
		if(feat.empty()) patches["normal"].reshape(1,1).copyTo(feat);
		else hconcat(patches["normal"].reshape(1,1), feat, feat);
	}
	if(patches.find("depth") != patches.end()) {
		//if(feat.empty()) patches["normal"].reshape(1,1).copyTo(feat);
		//else 
		Scalar mean_, std_;
		//normalize(patches["depth"], patches["depth"], 1, 0, NORM_MINMAX);
		meanStdDev(patches["depth"], mean_, std_);
		Mat patch_ = (patches["depth"]-mean_.val[0]);///std_.val[0];
		hconcat(patch_.reshape(1,1), feat, feat);
	}

	return true;
}

// extract patches from database images
bool ObjPatchMatcher::PreparePatchDB(DatasetName db_name) {

	patch_meta.objects.clear();
	cout<<patch_meta.objects.max_size()<<endl;

	DataManagerInterface* db_man = NULL;
	if(db_name == DB_NYU2_RGBD)
		db_man = new NYUDepth2DataMan;
	if(db_name == DB_SALIENCY_RGBD)
		db_man = new RGBDECCV14;

	srand(time(0));
	FileInfos imgfns, dmapfns;
	db_man->GetImageList(imgfns);
	random_shuffle(imgfns.begin(), imgfns.end());
	imgfns.erase(imgfns.begin()+30, imgfns.end());
	db_man->GetDepthmapList(imgfns, dmapfns);
	map<string, vector<VisualObject>> gt_masks;
	db_man->LoadGTMasks(imgfns, gt_masks);

	int gt_obj_cnt = 0;
	gt_obj_masks.clear();

	for(size_t i=0; i<imgfns.size(); i++) {
		// color
		Mat cimg = imread(imgfns[i].filepath);
		Size newsz(400, 400);
		//tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
		resize(cimg, cimg, newsz);
		// depth
		Mat dmap_float;
		db_man->LoadDepthData(dmapfns[i].filepath, dmap_float);
		resize(dmap_float, dmap_float, newsz);
		dmap_float.convertTo(dmap_float, CV_32F, 1000);
		Mat cmp_mask;
		compare(dmap_float, 800, cmp_mask, CMP_LT);
		dmap_float.setTo(800, cmp_mask);
		compare(dmap_float, 7000, cmp_mask, CMP_GT);
		dmap_float.setTo(7000, cmp_mask);
		dmap_float = (dmap_float-800)/(7000-800);
		
		// get label image
		Mat lable_mask = Mat::zeros(newsz.height, newsz.width, CV_8U);
		Mat label_id_mask = Mat::zeros(newsz.height, newsz.width, CV_32S)-1;
		vector<VisualObject>& gt_objs = gt_masks[imgfns[i].filename];
		for(auto& cur_gt : gt_objs) {
			//if( !valid_cls[cur_gt.category_id] ) continue;
			resize(cur_gt.visual_desc.mask, cur_gt.visual_desc.mask, newsz);
			label_id_mask.setTo(gt_obj_cnt++, cur_gt.visual_desc.mask);
			gt_obj_masks.push_back(cur_gt.visual_desc.mask);
			lable_mask.setTo(1, cur_gt.visual_desc.mask);
		}

		imshow("color", cimg);
		ImgVisualizer::DrawFloatImg("depth", dmap_float);
		imshow("label", lable_mask*255);
		waitKey(10);

		// do edge detection to locate boundary point quickly
		Mat gray_img, edge_map, gray_img_float;
		cvtColor(cimg, gray_img, CV_BGR2GRAY);
		gray_img.convertTo(gray_img_float, CV_32F, 1.f/255);
		Canny(gray_img, edge_map, 10, 50);
		Mat grad_x, grad_y, grad_mag;
		Sobel(gray_img_float, grad_x, CV_32F, 1, 0);
		Sobel(gray_img_float, grad_y, CV_32F, 0, 1);
		magnitude(grad_x, grad_y, grad_mag);
		Mat pts3d, normal_map;
		if(use_depth) {
			Feature3D feat3d;
			feat3d.ComputeKinect3DMap(dmap_float, pts3d, false);
			feat3d.ComputeNormalMap(pts3d, normal_map);
		}

		// selected patch indicator, avoid very close duplicate patches
		Mat picked_patch_mask = Mat::zeros(lable_mask.rows, lable_mask.cols, CV_8U);

		// extract patches
		for(int r=patch_size.height/2; r<edge_map.rows-patch_size.height/2; r+=3) {
			for(int c=patch_size.width/2; c<edge_map.cols-patch_size.width/2; c+=3) {

				if( edge_map.at<uchar>(r,c) == 0 ) continue;

				// only use perfect boundary points
				Point center_pt(c, r), left_pt(c-1, r), right_pt(c+1, r), top_pt(c, r-1), bottom_pt(c, r+1);
				if(lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(left_pt) && 
					lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(right_pt) &&
					lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(top_pt) &&
					lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(bottom_pt) );
					//continue;

				// set picked
				picked_patch_mask.at<uchar>(center_pt) = 1;

				VisualObject cur_patch;
				cur_patch.imgpath = imgfns[i].filepath;
				Rect box(c-patch_size.width/2, r-patch_size.height/2, patch_size.width, patch_size.height);
				cur_patch.visual_desc.box = box;

				// find which object is dominant
				map<int, int> obj_label_cnt;
				int max_num = 0;
				cur_patch.category_id = -1;
				for(int rr=box.y; rr<box.br().y; rr++) for(int cc=box.x; cc<box.br().x; cc++) {
					int cur_id = label_id_mask.at<int>(rr,cc);
					if(cur_id != -1)
					{
						obj_label_cnt[cur_id]++;
						if(obj_label_cnt[cur_id] > max_num) 
						{ 
							max_num = obj_label_cnt[cur_id];
							cur_patch.category_id = cur_id;
						}
					}
				}
				/*vector<ImgWin> boxes; boxes.push_back(box);
				ImgVisualizer::DrawWinsOnImg("", cimg, boxes);
				if(cur_patch.category_id != -1) {
				imshow("mask", gt_obj_masks[cur_patch.category_id]*255);
				waitKey(0);
				}*/
				

				lable_mask(box).convertTo(cur_patch.visual_desc.mask, CV_32F);
				// extract feature vector
				gray_img_float(box).copyTo( cur_patch.visual_desc.extra_features["gray"] );
				//grad_mag(box).copyTo( cur_patch.visual_desc.extra_features["gradient"] );
				if(use_depth) {
					normal_map(box).copyTo( cur_patch.visual_desc.extra_features["normal"] );
					dmap_float(box).copyTo( cur_patch.visual_desc.extra_features["depth"] );
					/*ImgVisualizer::DrawFloatImg("depthmask", cur_patch.visual_desc.extra_features["depth"]);
					cout<<"new box"<<endl;
					waitKey(0);*/
				}
				Mat feat;
				ComputePatchFeat(cur_patch.visual_desc.extra_features, feat);
				patch_data.push_back(feat);
				patch_meta.objects.push_back(cur_patch);
			}
				
				//}
			//}
		}
		
		cout<<"finished image: "<<i<<"/"<<imgfns.size()<<endl;
		cout<<"db size: "<<patch_data.rows<<endl;
	}

	cout<<"total patch number: "<<patch_data.rows<<endl;

	if(use_code) {
		// compress to codes
		LSHCoder lsh_coder;
		if( !lsh_coder.LearnOptimalCodes(patch_data, 64, patch_keys) ) {
			return false;
		}
	}

	return true;
}

bool ObjPatchMatcher::PrepareViewPatchDB() {

	// get files
	patch_data.release();
	patch_meta.objects.clear();
	DirInfos cate_dirs;
	// top categories
	ToolFactory::GetDirsFromDir(uw_view_root, cate_dirs);	
	cate_dirs.erase(cate_dirs.begin(), cate_dirs.begin()+5);
	cate_dirs.erase(cate_dirs.begin()+1, cate_dirs.end());
	for(size_t i=0; i<cate_dirs.size(); i++) {
		DirInfos sub_cate_dirs;
		ToolFactory::GetDirsFromDir(cate_dirs[i].dirpath, sub_cate_dirs);
		for(auto cur_sub_dir : sub_cate_dirs) {
			FileInfos cate_fns;
			ToolFactory::GetFilesFromDir(cur_sub_dir.dirpath, "*_crop.png", cate_fns);
			for (auto cur_fn : cate_fns) {
				VisualObject cur_obj_view;
				cur_obj_view.category_id = i;
				cur_obj_view.imgpath = cur_fn.filepath;
				cur_obj_view.imgfile = cur_fn.filename;
				cur_obj_view.dmap_path = cur_fn.filepath.substr(0, cur_fn.filepath.length()-7) + "depthcrop.png";
				patch_meta.objects.push_back(cur_obj_view);
			}
		}

		cout<<"Loaded category: "<<cate_dirs[i].dirpath<<endl;
	}

	// get features
	cout<<"Extracting view features..."<<endl;
	for(size_t i=0; i<patch_meta.objects.size(); i++) {
		VisualObject& cur_obj = patch_meta.objects[i];

		Mat vimg = imread(cur_obj.imgpath);
		//Mat dmap = imread(cur_obj_view.dmap_path, CV_LOAD_IMAGE_UNCHANGED);
		resize(vimg, vimg, patch_size);
		Mat gray_img_float, grad_x, grad_y, grad_mag;
		cvtColor(vimg, gray_img_float, CV_BGR2GRAY);
		gray_img_float.convertTo(gray_img_float, CV_32F);
		Sobel(gray_img_float, grad_x, CV_32F, 1, 0);
		Sobel(gray_img_float, grad_y, CV_32F, 0, 1);
		magnitude(grad_x, grad_y, grad_mag);
		grad_mag.copyTo(cur_obj.visual_desc.extra_features["gradient"]);

		Mat cur_feat;
		ComputePatchFeat(cur_obj.visual_desc.extra_features, cur_feat);
		patch_data.push_back(cur_feat);

		cout<<i<<"/"<<patch_meta.objects.size()<<endl;
	}

	cout<<"Feature extraction done."<<endl;

	cout<<"Database ready."<<endl;

	return true;

}

bool ObjPatchMatcher::Match(const Mat& cimg, const Mat& dmap_raw, Mat& mask_map) {

	/*
	 * precompute feature maps
	 */
	// gradient
	Mat gray_img, gray_img_float, edge_map;
	cvtColor(cimg, gray_img, CV_BGR2GRAY);
	gray_img.convertTo(gray_img_float, CV_32F, 1.f/255);
	Canny(gray_img, edge_map, 10, 50);
	imshow("edge", edge_map);
	imshow("color", cimg);
	waitKey(10);

	Mat grad_x, grad_y, grad_mag;
	Sobel(gray_img_float, grad_x, CV_32F, 1, 0);
	Sobel(gray_img_float, grad_y, CV_32F, 0, 1);
	magnitude(grad_x, grad_y, grad_mag);

	// depth
	Mat dmap_float, pts3d, normal_map;
	if( use_depth ) {
		Feature3D feat3d;
		dmap_raw.convertTo(dmap_float, CV_32F);
		Mat cmp_mask;
		compare(dmap_float, 800, cmp_mask, CMP_LT);
		dmap_float.setTo(800, cmp_mask);
		compare(dmap_float, 7000, cmp_mask, CMP_GT);
		dmap_float.setTo(7000, cmp_mask);
		dmap_float = (dmap_float-800)/(7000-800);

		feat3d.ComputeKinect3DMap(dmap_float, pts3d, false);
		feat3d.ComputeNormalMap(pts3d, normal_map);
	}

	/*
	 *	start searching
	 */
	// init searcher
	//searcher.Build(patch_data, BruteForce_L2);	// opencv bfmatcher has size limit: maximum 2^31
	LSHCoder lsh_coder;
	if(use_code) {
		lsh_coder.Load();
	}
	
	Mat score_map = Mat::zeros(edge_map.rows, edge_map.cols, CV_32F);
	Mat mask_vote_map = Mat::zeros(cimg.rows, cimg.cols, CV_32F);
	mask_map = Mat::zeros(cimg.rows, cimg.cols, CV_32F);
	Mat mask_count = Mat::zeros(cimg.rows, cimg.cols, CV_32S);	// number of mask overlapped on each pixel
	Mat feat;
	int topK = 40;
	int total_cnt = countNonZero(edge_map);
	vector<VisualObject> query_patches;
	query_patches.reserve(total_cnt);

	cout<<"Start match..."<<endl;
	
	float max_dist = 0;
	int cnt = 0;
	char str[30];
	double start_t = getTickCount();
//#pragma omp parallel for
	for(int r=patch_size.height/2; r<gray_img.rows-patch_size.height/2; r+=3) {
		for(int c=patch_size.width/2; c<gray_img.cols-patch_size.width/2; c+=3) {

			/*int rand_r = rand()%gray_img.rows;
			int rand_c = rand()%gray_img.cols;
			if(rand_r < patch_size.height/2 || rand_r > gray_img.rows-patch_size.height/2 ||
				rand_c < patch_size.width/2 || rand_c > gray_img.cols-patch_size.width/2) continue;*/
			int rand_r = r, rand_c = c;

			if(edge_map.at<uchar>(rand_r, rand_c) > 0) 
			{
				cnt++;
				destroyAllWindows();

				Rect box(rand_c-patch_size.width/2, rand_r-patch_size.height/2, patch_size.width, patch_size.height);
				MatFeatureSet featset;
				gray_img_float(box).copyTo(featset["gray"]);
				//grad_mag(box).copyTo(featset["gradient"]);
				if(use_depth)
				{ 
					normal_map(box).copyTo(featset["normal"]);
					dmap_float(box).copyTo(featset["depth"]);
				}
				ComputePatchFeat(featset, feat);
				vector<DMatch> matches;
				if(use_code) 
				{
					BinaryCodes codes;
					HashKey key_val;
					lsh_coder.ComputeCodes(feat, codes);
					HashingTools<HashKeyType>::CodesToKey(codes, key_val);
					MatchCode(key_val, topK, matches);
				}
				else
				{
					MatchPatch(feat, topK, matches);
				}
				
				if(matches[0].distance < 0 || matches[0].distance > 1000) {
					cout<<"match dist: "<<matches[0].distance<<endl;
					double minv, maxv;
					cout<<norm(feat, patch_data.row(matches[0].trainIdx), NORM_L2)<<endl;
					minMaxLoc(feat, &minv, &maxv);
					cout<<minv<<" "<<maxv<<endl;
					cout<<feat<<endl<<endl;
					minMaxLoc(patch_data.row(matches[0].trainIdx), &minv, &maxv);
					cout<<minv<<" "<<maxv<<endl;
					cout<<patch_data.row(matches[0].trainIdx)<<endl;
					imshow("cimg", cimg);
					waitKey(0);
				}
				vector<vector<Mat>> pixel_mask_vals(patch_size.height, vector<Mat>(patch_size.width, Mat::zeros(1, topK, CV_32F)));
				VisualObject cur_query;
				cur_query.visual_desc.box = box;
				cur_query.visual_desc.mask = Mat::zeros(patch_size.height, patch_size.width, CV_32F);
				for(size_t i=0; i<topK; i++) { 
					score_map.at<float>(rand_r,rand_c) += matches[i].distance;
					cur_query.visual_desc.mask += patch_meta.objects[matches[i].trainIdx].visual_desc.mask;
					for(int mr=0; mr<patch_size.height; mr++) for(int mc=0; mc<patch_size.width; mc++) {
						pixel_mask_vals[mr][mc].at<float>(i) = 
							patch_meta.objects[matches[i].trainIdx].visual_desc.mask.at<float>(mr, mc);
					}
				}
				score_map.at<float>(rand_r,rand_c) /= topK;
				cur_query.visual_desc.mask /= topK;			// average returned mask
				
				// compute mask quality
				Scalar mean_, std_;
				/*ofstream out("pixel_mask_std_100.txt", ios::app);
				for(int mr=0; mr<patch_size.height; mr++) for(int mc=0; mc<patch_size.width; mc++) {
				meanStdDev(pixel_mask_vals[mr][mc], mean_, std_);
				out<<std_.val[0]<<" ";
				}
				out<<endl;*/
				meanStdDev(cur_query.visual_desc.mask, mean_, std_);
				cur_query.visual_desc.scores.push_back(mean_.val[0]);
				cur_query.visual_desc.scores.push_back(std_.val[0]);

				Mat align_mask = Mat::zeros(cimg.rows, cimg.cols, CV_8U);
				int gt_mask_id = patch_meta.objects[matches[0].trainIdx].category_id;
				if(gt_mask_id != -1) {
					Mat nn_mask = gt_obj_masks[gt_mask_id];
					//imshow("gt mask", nn_mask*255);
					//waitKey(10);
					Rect gt_box = patch_meta.objects[matches[0].trainIdx].visual_desc.box;
					Rect align_box = AlignBox(box, gt_box, cimg.cols, cimg.rows);
					vector<ImgWin> boxes; boxes.push_back(align_box);
					//ImgVisualizer::DrawWinsOnImg("alignbox", cimg, boxes);
					//waitKey(10);
					Rect target_box = Rect(box.x-(gt_box.x-align_box.x), box.y-(gt_box.y-align_box.y), align_box.width, align_box.height);
					cout<<target_box<<endl;
					nn_mask(align_box).copyTo(align_mask(target_box));
				}
				align_mask.convertTo(align_mask, CV_32F);
				mask_map += align_mask * matches[0].distance;	//*score_map.at<float>(r,c);
				//mask_count(box) = mask_count(box) + 1;

				//cout<<score_map.at<float>(r,c)<<endl;
				max_dist = MAX(max_dist, matches[0].distance);
				query_patches.push_back(cur_query);

				// vote object regions
				/*Point3f line_ori;
				int obj_pt_sign;
				ComputeDominantLine(cur_query.visual_desc.mask, box.tl(), line_ori, obj_pt_sign);
				for(int rr=0; rr<cimg.rows; rr++) for(int cc=0; cc<cimg.cols; cc++) {
				float line_val = line_ori.x*cc+line_ori.y*rr+line_ori.z;
				if((line_val>0?1:-1)==obj_pt_sign) mask_vote_map.at<float>(rr, cc)++;
				}*/

#ifdef VERBOSE

				// current patch
				Mat disp, patch_gray, patch_grad, patch_normal, patch_depth;
				disp = cimg.clone();
				rectangle(disp, box, CV_RGB(255,0,0), 2);
				resize(gray_img(box), patch_gray, Size(50,50));
				resize(grad_mag(box), patch_grad, Size(50,50));
				Mat cur_mask;
				resize(cur_query.visual_desc.mask, cur_mask, Size(50,50));
				if(use_depth) 
				{
					resize(normal_map(box), patch_normal, Size(50,50));

					normalize(dmap_float(box), patch_depth, 1, 0, NORM_MINMAX);
					patch_depth.convertTo(patch_depth, CV_8U, 255);
					//dmap_float(box).convertTo(patch_depth, CV_8U, 255);
					resize(patch_depth, patch_depth, Size(50,50));
				}

				Mat onormal;
				sprintf_s(str, "query_gray_%d.jpg", cnt);
				imshow(str, patch_gray);
				imwrite(str, patch_gray);

				/*sprintf_s(str, "query_grad_%d.jpg", cnt);
				ImgVisualizer::DrawFloatImg(str, patch_grad, onormal, true);
				imwrite(str, onormal);*/
				
				sprintf_s(str, "query_depth_%d.jpg", cnt);
				imshow(str, patch_depth);
				imwrite(str, patch_depth);
				
				sprintf_s(str, "query_normal_%d.jpg", cnt);
				ImgVisualizer::DrawNormals(str, patch_normal, onormal, true);
				imwrite(str, onormal);

				sprintf_s(str, "query_box_%d.jpg", cnt);
				imshow(str, disp);
				imwrite(str, disp);

				//imshow("align mask", align_mask*255);

				cur_mask.convertTo(cur_mask, CV_8U, 255);
				sprintf_s(str, "query_tmask_%d.jpg", cnt);
				imshow(str, cur_mask);
				imwrite(str, cur_mask);

				// show match results
				vector<Mat> res_imgs(topK);
				vector<Mat> res_gradients(topK);
				vector<Mat> res_normals(topK);
				vector<Mat> res_depth(topK);
				vector<Mat> db_boxes(topK);
				vector<Mat> res_masks(topK);
				for(size_t i=0; i<topK; i++) {
					VisualObject& cur_obj = patch_meta.objects[matches[i].trainIdx];
					// mask
					cur_obj.visual_desc.mask.convertTo(res_masks[i], CV_8U, 255);
					// gray
					cur_obj.visual_desc.extra_features["gray"].convertTo(res_imgs[i], CV_8U, 255);
					// gradient
					//ImgVisualizer::DrawFloatImg("", cur_obj.visual_desc.extra_features["gradient"], res_gradients[i], false);
					// 3D
					if(use_depth) 
					{
						// normal
						tools::ImgVisualizer::DrawNormals("", cur_obj.visual_desc.extra_features["normal"], res_normals[i]);
						// depth
						normalize(cur_obj.visual_desc.extra_features["depth"], res_depth[i], 1, 0, NORM_MINMAX);
						res_depth[i].convertTo(res_depth[i], CV_8U, 255);
						//cur_obj.visual_desc.extra_features["depth"].convertTo(res_depth[i], CV_8U, 255);
					}
					// box on image
					db_boxes[i] = imread(patch_meta.objects[matches[i].trainIdx].imgpath);
					resize(db_boxes[i], db_boxes[i], Size(cimg.cols, cimg.rows));
					rectangle(db_boxes[i], patch_meta.objects[matches[i].trainIdx].visual_desc.box, CV_RGB(255,0,0), 2);
				}
				Mat out_img;
				sprintf_s(str, "res_gray_%d.jpg", cnt);
				ImgVisualizer::DrawImgCollection(str, res_imgs, topK, Size(50,50), out_img);
				imwrite(str, out_img);
				
				sprintf_s(str, "res_normal_%d.jpg", cnt);
				ImgVisualizer::DrawImgCollection(str, res_normals, topK, Size(50,50), out_img);
				imwrite(str, out_img);

				sprintf_s(str, "res_depth_%d.jpg", cnt);
				ImgVisualizer::DrawImgCollection(str, res_depth, topK, Size(50,50), out_img);
				imwrite(str, out_img);

				/*sprintf_s(str, "res_gradient_%d.jpg", cnt);
				tools::ImgVisualizer::DrawImgCollection(str, res_gradients, topK, Size(50,50), out_img);
				imwrite(str, out_img);*/

				sprintf_s(str, "res_mask_%d.jpg", cnt);
				tools::ImgVisualizer::DrawImgCollection(str, res_masks, topK, Size(50,50), out_img);
				imwrite(str, out_img);

				sprintf_s(str, "res_box_%d.jpg", cnt);
				tools::ImgVisualizer::DrawImgCollection(str, db_boxes, topK/2, Size(200, 200), out_img);
				imwrite(str, out_img);

				waitKey(0);
#endif

				cout<<total_cnt--<<endl;
			}
		}
	}
	cout<<"match done. Time cost: "<<(getTickCount()-start_t)/getTickFrequency()<<"s."<<endl;

	//score_map(Rect(patch_size.width/2, patch_size.height/2, score_map.cols-patch_size.width/2, score_map.rows-patch_size.height/2)).copyTo(score_map);
	//score_map.setTo(max_dist, 255-edge_map);
	normalize(score_map, score_map, 1, 0, NORM_MINMAX);
	score_map = 1-score_map;
	//tools::ImgVisualizer::DrawFloatImg("bmap", score_map);

	mask_map /= max_dist;
	cout<<max_dist<<endl;
	normalize(mask_map, mask_map, 1, 0, NORM_MINMAX);
	//tools::ImgVisualizer::DrawFloatImg("maskmap", mask_map);

	//normalize(mask_vote_map, mask_vote_map, 1, 0, NORM_MINMAX);
	//ImgVisualizer::DrawFloatImg("vote map", mask_vote_map);
	//waitKey(0);

	return true;

	// pick top weighted points to see if they are inside objects
	// try graph-cut for region proposal
	// among all retrieved mask patch, select most discriminative one and do graph-cut
	sort(query_patches.begin(), query_patches.end(), [](const VisualObject& a, const VisualObject& b) { return a.visual_desc.scores[1] > b.visual_desc.scores[1]; });
	for(size_t i=0; i<query_patches.size(); i++) {
		Mat disp_img = cimg.clone();
		rectangle(disp_img, query_patches[i].visual_desc.box, CV_RGB(255,0,0));
		imshow("max std box", disp_img);
		Mat big_mask;
		resize(query_patches[i].visual_desc.mask, big_mask, Size(50,50));
		ImgVisualizer::DrawFloatImg("max std mask", big_mask);
		waitKey(0);
		// use mask to do graph-cut
		Mat fg_mask(cimg.rows, cimg.cols, CV_8U);
		fg_mask.setTo(cv::GC_PR_FGD);
		Mat th_mask;
		threshold(query_patches[i].visual_desc.mask, th_mask, query_patches[i].visual_desc.scores[0], 1, CV_THRESH_BINARY);
		th_mask.convertTo(th_mask, CV_8U);
		fg_mask(query_patches[i].visual_desc.box).setTo(cv::GC_FGD, th_mask);
		th_mask = 1-th_mask;
		fg_mask(query_patches[i].visual_desc.box).setTo(cv::GC_BGD, th_mask);
		cv::grabCut(cimg, fg_mask, Rect(0,0,1,1), Mat(), Mat(), 3, cv::GC_INIT_WITH_MASK);
		fg_mask = fg_mask & 1;
		disp_img.setTo(Vec3b(0,0,0));
		cimg.copyTo(disp_img, fg_mask);
		imshow("cut", disp_img);
		waitKey(0);
	}


	float ths[] = {0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f};
	for(size_t i=0; i<8; i++) {
		Mat th_mask;
		threshold(mask_map, th_mask, ths[i], 1, CV_THRESH_BINARY);
		char str[30];
		sprintf_s(str, "%f", ths[i]);
		ImgVisualizer::DrawFloatImg(str, th_mask);
		waitKey(0);
	}

	return true;
}

bool ObjPatchMatcher::MatchViewPatch(const Mat& cimg, const Mat& dmap_raw) {

	// compute feature map
	Mat gray_img, gray_img_float;
	cvtColor(cimg, gray_img, CV_BGR2GRAY);
	gray_img.convertTo(gray_img_float, CV_32F);
	waitKey(10);

	// gradient
	Mat grad_x, grad_y, grad_mag;
	Sobel(gray_img_float, grad_x, CV_32F, 1, 0);
	Sobel(gray_img_float, grad_y, CV_32F, 0, 1);
	magnitude(grad_x, grad_y, grad_mag);

	// depth

	// do search
	Mat score_map = Mat::zeros(cimg.rows, cimg.cols, CV_32F);
	Mat feat;
	int topK = 1;
	map<float, ImgWin> top_det;

	cout<<"Start match..."<<endl;
	float max_dist = 0;
	double start_t = getTickCount();
	//#pragma omp parallel for
	for(int r=patch_size.height/2; r<gray_img.rows-patch_size.height/2; r++) {
		for(int c=patch_size.width/2; c<gray_img.cols-patch_size.width/2; c++) {

			Rect box(c-patch_size.width/2, r-patch_size.height/2, patch_size.width, patch_size.height);
			MatFeatureSet featset;
			grad_mag(box).copyTo(featset["gradient"]);
			ComputePatchFeat(featset, feat);
			vector<DMatch> matches;
			MatchPatch(feat, topK, matches);

			for(size_t i=0; i<topK; i++) { 
				score_map.at<float>(r,c) += matches[i].distance;
			}
			score_map.at<float>(r,c) /= topK;

			ImgWin best_match = box;
			best_match.score = matches[0].trainIdx;
			top_det[matches[0].distance] = best_match;
			max_dist = MAX(max_dist, score_map.at<float>(r,c));
		}
	}

	imshow("color", cimg);
	ImgVisualizer::DrawFloatImg("scoremap", score_map);
	waitKey(0);

	int cnt = 0;
	Mat disp_img = cimg.clone();
	for(auto pi=top_det.begin(); pi!=top_det.end(); pi++) {
		cout<<pi->first<<endl;
		rectangle(disp_img, pi->second, CV_RGB(255,0,0), 2);
		Mat db_view = imread(patch_meta.objects[pi->second.score].imgpath);
		resize(db_view, db_view, patch_size);
		Mat test_win;
		resize(cimg(pi->second), test_win, patch_size);
		ImgVisualizer::DrawFloatImg("db grad", patch_meta.objects[pi->second.score].visual_desc.img_desc);
		ImgVisualizer::DrawFloatImg("test grad", grad_mag(pi->second));
		imshow("db view", db_view);
		imshow("color", disp_img);
		waitKey(0);
		if(++cnt == 100) break;
	}

	return true;
}

bool ObjPatchMatcher::MatchPatch(const Mat& feat, int k, vector<DMatch>& res) {

	res.resize(patch_data.rows);
#pragma omp parallel for
	for(int r=0; r<patch_data.rows; r++) {
		res[r].trainIdx = r;
		res[r].distance = norm(feat, patch_data.row(r), NORM_L2);
	}
	//nth_element(res.begin(), res.begin()+k, res.end(), [](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });
	//partition(res.begin(), res.end(), [&](const DMatch& a) { return a.distance <= res[k].distance; });
	partial_sort(res.begin(), res.begin()+k, res.end(), [](const DMatch& a, const DMatch& b) { return a.distance<b.distance; } );

	return true;
}

bool ObjPatchMatcher::MatchCode(const HashKey& query_key, int k, vector<DMatch>& res) {
	
	if( patch_keys.empty() ) return false;
	res.resize(patch_keys.size());
#pragma omp parallel for
	for(int i=0; i<patch_keys.size(); i++) {
		res[i].trainIdx = i;
		res[i].distance = HashingTools<HashKeyType>::HammingDist(query_key, patch_keys[i]);
	}
	//for(auto val : res) cout<<val.distance<<endl;
	nth_element(res.begin(), res.begin()+k, res.end(), [](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });
	partition(res.begin(), res.end(), [&](const DMatch& a) { return a.distance < res[k].distance; });

	return true;
}

bool ObjPatchMatcher::ComputeDominantLine(const Mat& mask_patch, Point tl_pt, Point3f& line_coeff, int& obj_pt_sign) {
	Mat grad_x, grad_y, grad_mag;
	Sobel(mask_patch, grad_x, CV_32F, 1, 0);
	Sobel(mask_patch, grad_y, CV_32F, 0, 1);
	magnitude(grad_x, grad_y, grad_mag);

	float mean_grad_x = mean(grad_x).val[0];
	float mean_grad_y = mean(grad_y).val[0];
	// line: y=kx+b; k=-meanx/meany + pass center
	Point centerp(tl_pt.x+mask_patch.cols/2, tl_pt.y+mask_patch.rows/2);
	float k = -mean_grad_x / mean_grad_y;
	float b = centerp.y - k*centerp.x;
	line_coeff.x = k;
	line_coeff.y = -1;
	line_coeff.z = b;

	float obj_pos = 0;
	Point tl(centerp.x-mask_patch.cols/2, centerp.y-mask_patch.rows/2);
	for(int r=0; r<mask_patch.rows; r++) for(int c=0; c<mask_patch.cols; c++) {
		obj_pos += mask_patch.at<float>(r,c)*(line_coeff.x*(tl.x+c)+line_coeff.y*(tl.y+r)+line_coeff.z>0? 1:-1);
	}
	obj_pt_sign = obj_pos>0? 1: -1;

	return true;
}

bool ObjPatchMatcher::PatchDataIO(bool toSave) {
	string fn = "patch.data";
	if(toSave)
	{
		ofstream out(fn);
		// save patch meta
		for(size_t i=0; i<patch_meta.objects.size(); i++) {
			// patch location
			
		}
		// save patch feature
		for(int i=0; i<patch_data.rows; i++) {

		}
	}
	else
	{
		ifstream in(fn);
	}
	return true;
}

Rect ObjPatchMatcher::AlignBox(Rect box1, Rect box2, int imgw, int imgh) {

	Rect newbox;
	int minleft = MIN(box1.x, box2.x);
	int mintop = MIN(box1.y, box2.y);
	int minright = MIN(imgw-box1.x-1, imgw-box2.x-1);
	int minbottom = MIN(imgh-box1.y-1, imgh-box2.y-1);
	newbox.x = MAX(0, box2.x-minleft);
	newbox.y = MAX(0, box2.y-mintop);
	newbox.width = minright+minleft-2;
	newbox.height = minbottom+mintop-2;
	if(newbox.width <0 || newbox.height<0)
		cout<<"error"<<endl;
	return newbox;
}