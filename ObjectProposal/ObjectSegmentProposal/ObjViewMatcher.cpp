#include "ObjViewMatcher.h"


ObjViewMatcher::ObjViewMatcher(void)
{
	root_dir = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\";
	view_sz = Size(30, 30);
}


bool ObjViewMatcher::PrepareDatabase() {
	// get files
	obj_db.objects.clear();
	DirInfos cate_dirs;
	// top categories
	ToolFactory::GetDirsFromDir(root_dir, cate_dirs);	
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
				obj_db.objects.push_back(cur_obj_view);
			}
		}

		cout<<"Loaded category: "<<cate_dirs[i].dirpath<<endl;
	}

	// get features
	cout<<"Extracting view features..."<<endl;
	for(size_t i=0; i<obj_db.objects.size(); i++) {
		Mat vimg = imread(obj_db.objects[i].imgpath);
		//Mat dmap = imread(cur_obj_view.dmap_path, CV_LOAD_IMAGE_UNCHANGED);
		resize(vimg, vimg, view_sz);
		//resize(dmap, dmap, view_sz);
		ExtractViewFeat(vimg, Mat(), obj_db.objects[i].visual_desc.img_desc);
		cout<<i<<"/"<<obj_db.objects.size()<<endl;
	}
	
	cout<<"Feature extraction done."<<endl;

	cout<<"Database ready."<<endl;

	return true;
}

bool ObjViewMatcher::LearnOptimalBinaryCodes(int code_len) {
	// generate lsh functions
	lsh_coder.GenerateHashFunctions(25*25, 128, true);
	// generate random weights for binary codes
	//lsh_coder.ComputeCodes(cur_obj.visual_desc.img_desc, cur_obj.visual_desc.binary_code);
	//HashingTools<HashKeyType>::CodesToKey(cur_obj.visual_desc.binary_code, cur_obj.visual_desc.key_value);
	
	return true;
}

float ObjViewMatcher::EvaluateCodeQuality() {
	float cost = 0;
	int cnt = 0;
	// pair-wise mean hamming distance
	for(size_t i=0; i<obj_db.objects.size(); i++) {
		for(size_t j=i+1; j<obj_db.objects.size(); j++) {
			cost += HashingTools<HashKeyType>::HammingDist(obj_db.objects[i].visual_desc.key_value, obj_db.objects[j].visual_desc.key_value);
			cnt++;
		}
	}
	cost /= cnt;
	return cost;
}

bool ObjViewMatcher::ExtractViewFeat(const Mat& color_view, const Mat& dmap_view, Mat& view_feat) {

	Mat gray_view;
	cvtColor(color_view, gray_view, CV_BGR2GRAY);
	edge_proc.Init(gray_view, EdgeFeatParams());
	edge_proc.Compute2(Mat(), view_feat);

	return true;
}

bool ObjViewMatcher::ComputeGradient(const Mat& color_view, Mat& grad_view) {
	Mat gray_view;
	cvtColor(color_view, gray_view, CV_BGR2GRAY);
	gray_view.convertTo(gray_view, CV_32F);
	Mat view_grad_x, view_grad_y;
	Sobel(gray_view, view_grad_x, CV_32F, 1, 0);
	Sobel(gray_view, view_grad_y, CV_32F, 0, 1);
	grad_view = abs(view_grad_x) + abs(view_grad_y);
	normalize(grad_view, grad_view, 1, 0, NORM_L2);
	return true;
}

bool ObjViewMatcher::MatchView(const Mat& view_feat, vector<DMatch>& matches) {

	/*BinaryCodes codes;
	HashKey key_value;
	lsh_coder.ComputeCodes(view_feat, codes);
	HashingTools<int>::CodesToKey(codes, key_value);*/

	// search
	matches.resize(obj_db.objects.size());
#pragma omp parallel for
	for (int i=0; i<obj_db.objects.size(); i++) {
		matches[i].distance = (float)norm(view_feat, obj_db.objects[i].visual_desc.img_desc, NORM_L2);	//HashingTools<HashKeyType>::HammingDist(key_value, obj_db.objects[i].visual_desc.key_value);	
		matches[i].trainIdx = i;
	}
	partial_sort(matches.begin(), matches.begin()+10, matches.end(), [](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });

	//// visualize
	//char str[100];
	//for(size_t i=0; i<10; i++) {
	//	Mat img = imread(obj_db.objects[matches[i].trainIdx].imgpath);
	//	sprintf_s(str, "%d", i);
	//	imshow(str, img);
	//	waitKey(10);
	//}

	return true;
}

bool ObjViewMatcher::SearchImage(const Mat& cimg, const Mat& dmap_raw) {

	Mat gray_img;
	cvtColor(cimg, gray_img, CV_BGR2GRAY);

	SlideWinParams win_params(cimg.cols, cimg.rows, 0.05, 0.05, 1, 1, 1, 1);
	double start_t = getTickCount();
	for(size_t id=0; id<win_params.win_specs.size(); id++) {
		ImgWin cur_win = win_params.win_specs[id];
		map<float, ImgWin> top_det;
		Mat score_map(cimg.rows, cimg.cols, CV_32F);
		score_map.setTo(1);
//#pragma omp parallel for
		for(int r=0; r<cimg.rows-cur_win.height-1; r++) {
			for(int c=0; c<cimg.cols-cur_win.width-1; c++) {
				ImgWin win(c, r, cur_win.width, cur_win.height);
				Mat sub_view;
				ExtractViewFeat(cimg(win), Mat(), sub_view);
				vector<DMatch> matches;
				MatchView(sub_view, matches);
				//cout<<matches[0].distance<<endl;
				score_map.at<float>(win.y+win.height/2, win.x+win.width/2) = matches[0].distance;
				ImgWin best_match = win;
				best_match.score = matches[0].trainIdx;
				top_det[matches[0].distance] = best_match;
			}
		}

		normalize(score_map, score_map, 1, 0, NORM_MINMAX);
		score_map = 1 - score_map;
		cout<<"Search time: "<<(getTickCount()-start_t) / getTickFrequency()<<"s."<<endl;
		Mat disp_img = cimg.clone();
		imshow("color", disp_img);
		ImgVisualizer::DrawFloatImg("objectness", score_map);
		waitKey(0);

		int cnt = 0;
		for(auto pi=top_det.begin(); pi!=top_det.end(); pi++) {
			cout<<pi->first<<endl;
			rectangle(disp_img, pi->second, CV_RGB(255,0,0), 2);
			Mat db_view = imread(obj_db.objects[pi->second.score].imgpath);
			resize(db_view, db_view, view_sz);
			Mat test_win;
			resize(cimg(pi->second), test_win, view_sz);
			Mat db_grad, test_grad;
			ComputeGradient(test_win, test_grad);
			ComputeGradient(db_view, db_grad);
			ImgVisualizer::DrawFloatImg("db grad", db_grad);
			ImgVisualizer::DrawFloatImg("test grad", test_grad);
			imshow("db view", db_view);
			imshow("color", disp_img);
			waitKey(0);
			if(++cnt == 100) break;
		}

	}

	return true;
}