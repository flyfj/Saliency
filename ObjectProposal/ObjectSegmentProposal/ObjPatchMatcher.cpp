#include "ObjPatchMatcher.h"


ObjPatchMatcher::ObjPatchMatcher(void)
{
	patch_size = Size(7,7);
	int sel_cls[] = {58, 7, 124, 24, 136, 157, 19,88, 3, 83, 5, 344, 238, 13, 80, 89, 408, 49, 66};
	valid_cls.resize(900, false);
	for(auto id : sel_cls) valid_cls[id] = true;
}

// normalized value concatenation
// gray patch has problem when contrast of object patch is flipped, gradient is more robust
bool ObjPatchMatcher::ComputePatchFeat(const Mat& patch, Mat& feat) {
	Scalar mean_, std_;
	meanStdDev(patch, mean_, std_);
	Mat patch_ = (patch-mean_.val[0]);///std_.val[0];
	patch_.reshape(1, 1).copyTo(feat);

	return true;
}

// extract patches from database images
bool ObjPatchMatcher::PreparePatchDB(DatasetName db_name) {

	patch_meta.objects.clear();
	cout<<patch_meta.objects.max_size()<<endl;

	DataManagerInterface* db_man = NULL;
	if(db_name == DB_NYU2_RGBD)
		db_man = new NYUDepth2DataMan;

	FileInfos imgfns, dmapfns;
	db_man->GetImageList(imgfns);
	random_shuffle(imgfns.begin(), imgfns.end());
	imgfns.erase(imgfns.begin()+40, imgfns.end());
	map<string, vector<VisualObject>> gt_masks;
	db_man->LoadGTMasks(imgfns, gt_masks);

	for(size_t i=0; i<imgfns.size(); i++) {
		Mat cimg = imread(imgfns[i].filepath);
		Size newsz;
		tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
		resize(cimg, cimg, newsz);

		// get label image
		Mat lable_mask = Mat::zeros(newsz.height, newsz.width, CV_8U);
		vector<VisualObject>& gt_objs = gt_masks[imgfns[i].filename];
		for(auto& cur_gt : gt_objs) {
			if( !valid_cls[cur_gt.category_id] ) continue;
			resize(cur_gt.visual_desc.mask, cur_gt.visual_desc.mask, newsz);
			lable_mask.setTo(1, cur_gt.visual_desc.mask);
		}

		imshow("color", cimg);
		imshow("label", lable_mask*255);
		waitKey(10);

		// do edge detection to locate boundary point quickly
		Mat gray_img, edge_map, gray_img_float;
		cvtColor(cimg, gray_img, CV_BGR2GRAY);
		gray_img.convertTo(gray_img_float, CV_32F);
		Canny(gray_img, edge_map, 10, 50);
		Mat grad_x, grad_y, grad_mag;
		Sobel(gray_img_float, grad_x, CV_32F, 1, 0);
		Sobel(gray_img_float, grad_y, CV_32F, 0, 1);
		magnitude(grad_x, grad_y, grad_mag);

		for(int r=patch_size.height/2; r<edge_map.rows-patch_size.height/2; r++) {
			for(int c=patch_size.width/2; c<edge_map.cols-patch_size.width/2; c++) {
					
				// only use perfect boundary points
				Point center_pt(c, r), left_pt(c-1, r), right_pt(c+1, r), top_pt(c, r-1), bottom_pt(c, r+1);
				if(lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(left_pt) && 
					lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(right_pt) &&
					lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(top_pt) &&
					lable_mask.at<uchar>(center_pt) == lable_mask.at<uchar>(bottom_pt) )
					continue;

				//if(edge_map.at<uchar>(r,c) > 0) {
					VisualObject cur_patch;
					cur_patch.imgpath = imgfns[i].filepath;
					Rect box(c-patch_size.width/2, r-patch_size.height/2, patch_size.width, patch_size.height);
					cur_patch.visual_desc.box = box;
					double sum_label = sum(lable_mask(box)).val[0];
					if(sum_label < 10 || sum_label > 35) continue;

					/*cout<<sum_label<<endl;
					Mat patch_large;
					resize(gray_img(box), patch_large, Size(50, 50));
					imshow("patch_b", patch_large);
					vector<ImgWin> boxes;
					boxes.push_back(box);
					tools::ImgVisualizer::DrawWinsOnImg("patch", cimg, boxes);
					waitKey(0);*/

					lable_mask(box).copyTo(cur_patch.visual_desc.mask);
					// extract feature vector
					gray_img(box).copyTo( cur_patch.visual_desc.img_data );
					grad_mag(box).copyTo( cur_patch.visual_desc.img_desc );
					Mat feat;
					ComputePatchFeat(grad_mag(box), feat);
					patch_data.push_back(feat);
					patch_meta.objects.push_back(cur_patch);
				}
			//}
		}
		
		cout<<"finished image: "<<i<<"/"<<imgfns.size()<<endl;
		cout<<"db size: "<<patch_data.rows<<endl;
	}

	cout<<"total patch number: "<<patch_data.rows<<endl;

	return true;
}

bool ObjPatchMatcher::Match(const Mat& cimg, const Mat& dmap_raw) {

	// color
	Mat gray_img, gray_img_float, edge_map;
	cvtColor(cimg, gray_img, CV_BGR2GRAY);
	gray_img.convertTo(gray_img_float, CV_32F);
	Canny(gray_img, edge_map, 10, 50);
	imshow("edge", edge_map);
	waitKey(10);

	Mat grad_x, grad_y, grad_mag;
	Sobel(gray_img_float, grad_x, CV_32F, 1, 0);
	Sobel(gray_img_float, grad_y, CV_32F, 0, 1);
	magnitude(grad_x, grad_y, grad_mag);

	// depth
	Feature3D feat3d;
	Mat dmap_float, pts3d, normal_map;
	dmap_raw.convertTo(dmap_float, CV_32F);
	feat3d.ComputeKinect3DMap(dmap_float, pts3d, false);
	feat3d.ComputeNormalMap(pts3d, normal_map);

	// init searcher
	searcher.Build(patch_data, BruteForce_L2);
	
	Mat score_map = Mat::zeros(edge_map.rows, edge_map.cols, CV_32F);
	Mat feat;
	int topK = 20;

	cout<<"Start match..."<<endl;
	int total_cnt = countNonZero(edge_map);
	float max_dist = 0;
	double start_t = getTickCount();
//#pragma omp parallel for
	for(int r=patch_size.height/2; r<gray_img.rows-patch_size.height/2; r++) {
		for(int c=patch_size.width/2; c<gray_img.cols-patch_size.width/2; c++) {
			if(edge_map.at<uchar>(r,c) > 0) {
				Rect box(c-patch_size.width/2, r-patch_size.height/2, patch_size.width, patch_size.height);
				ComputePatchFeat(grad_mag(box), feat);
				vector<DMatch> matches;
				MatchPatch(feat, topK, matches);
				
				for(size_t i=0; i<topK; i++) { 
					score_map.at<float>(r,c) += matches[i].distance; 
				}
				score_map.at<float>(r,c) /= topK;
				//cout<<score_map.at<float>(r,c)<<endl;
				max_dist = MAX(max_dist, score_map.at<float>(r,c));

#ifdef VERBOSE
				// show match results
				vector<Mat> res_imgs(topK);
				vector<Mat> res_gradients(topK);
				vector<Mat> db_boxes(topK);
				vector<Mat> res_masks(topK);
				for(size_t i=0; i<topK; i++) {
					const VisualObject& cur_obj = patch_meta.objects[matches[i].trainIdx];
					res_imgs[i] = cur_obj.visual_desc.img_data;
					res_masks[i] = cur_obj.visual_desc.mask*255;
					tools::ImgVisualizer::DrawFloatImg("", cur_obj.visual_desc.img_desc, res_gradients[i], false); 
					db_boxes[i] = imread(patch_meta.objects[matches[i].trainIdx].imgpath);
					resize(db_boxes[i], db_boxes[i], Size(400,300));
					rectangle(db_boxes[i], patch_meta.objects[matches[i].trainIdx].visual_desc.box, CV_RGB(255,0,0), 2);
				}
				Mat disp, patch_img;
				disp = cimg.clone();
				rectangle(disp, box, CV_RGB(255,0,0), 2);
				resize(gray_img(box), patch_img, Size(50, 50));
				imshow("query", patch_img);
				imshow("query box", disp);
				//tools::ImgVisualizer::DrawFloatImg("grad", grad_mag(box));
				tools::ImgVisualizer::DrawImgCollection("res_patches", res_imgs, topK, Size(50,50), Mat());
				tools::ImgVisualizer::DrawImgCollection("res_gradients", res_gradients, topK, Size(50,50), Mat());
				tools::ImgVisualizer::DrawImgCollection("res_masks", res_masks, topK, Size(50,50), Mat());
				tools::ImgVisualizer::DrawImgCollection("res_imgs", db_boxes, topK/2, Size(200, 200), Mat());
				waitKey(0);
#endif

				cout<<total_cnt--<<endl;
			}
		}
	}
	cout<<"match done. Time cost: "<<(getTickCount()-start_t)/getTickFrequency()<<"s."<<endl;

	score_map.setTo(max_dist, 255-edge_map);
	normalize(score_map, score_map, 1, 0, NORM_MINMAX);
	score_map = 1-score_map;
	tools::ImgVisualizer::DrawFloatImg("bmap", score_map);

	return true;
}

bool ObjPatchMatcher::MatchPatch(const Mat& feat, int k, vector<DMatch>& res) {

	res.resize(patch_data.rows);
#pragma omp parallel for
	for(int r=0; r<patch_data.rows; r++) {
		res[r].trainIdx = r;
		res[r].distance = norm(feat, patch_data.row(r), NORM_L2);
	}
	partial_sort(res.begin(), res.begin()+k, res.end(), [](const DMatch& a, const DMatch& b) { return a.distance<b.distance; } );

	return true;
}