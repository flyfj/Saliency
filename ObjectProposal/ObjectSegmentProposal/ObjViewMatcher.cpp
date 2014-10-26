#include "ObjViewMatcher.h"


ObjViewMatcher::ObjViewMatcher(void)
{
	root_dir = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\";
}


bool ObjViewMatcher::PrepareDatabase() {
	// get files
	obj_db.objects.clear();
	DirInfos cate_dirs;
	ToolFactory::GetDirsFromDir(root_dir, cate_dirs);
	cate_dirs.erase(cate_dirs.begin()+10, cate_dirs.end());
	for(auto cur_dir : cate_dirs) {
		DirInfos sub_cate_dirs;
		ToolFactory::GetDirsFromDir(cur_dir.dirpath, sub_cate_dirs);
		for(auto cur_sub_dir : sub_cate_dirs) {
			FileInfos cate_fns;
			ToolFactory::GetFilesFromDir(cur_sub_dir.dirpath, "*_crop.png", cate_fns);
			for (auto cur_fn : cate_fns) {
				VisualObject cur_obj;
				cur_obj.imgpath = cur_fn.filepath;
				cur_obj.imgfile = cur_fn.filename;
				obj_db.objects.push_back(cur_obj);
			}
		}

		cout<<"Finish category: "<<cur_dir.dirpath<<endl;
	}

	// generate lsh functions
	lsh_coder.GenerateHashFunctions(25*25, 128, true);

	// get features
	cout<<"Extracting view features..."<<endl;
	for(auto& cur_obj : obj_db.objects) {
		Mat vimg = imread(cur_obj.imgpath);
		resize(vimg, vimg, Size(25, 25));
		ExtractViewFeat(vimg, cur_obj.visual_desc.img_desc);
		// compress
		lsh_coder.ComputeCodes(cur_obj.visual_desc.img_desc, cur_obj.visual_desc.binary_code);
		HashingTools<HashKeyType>::CodesToKey(cur_obj.visual_desc.binary_code, cur_obj.visual_desc.key_value);
	}
	cout<<"Feature extraction done."<<endl;

	cout<<"Database ready."<<endl;

	return true;
}

bool ObjViewMatcher::ExtractViewFeat(const Mat& color_view, Mat& view_feat) {
	Mat gray_view;
	cvtColor(color_view, gray_view, CV_BGR2GRAY);
	gray_view.convertTo(gray_view, CV_32F);
	Mat view_grad_x, view_grad_y;
	Sobel(gray_view, view_grad_x, CV_32F, 1, 0);
	Sobel(gray_view, view_grad_y, CV_32F, 0, 1);
	Mat view_grad_mag = abs(view_grad_x) + abs(view_grad_y);
	normalize(view_grad_mag, view_grad_mag, 1, 0, NORM_MINMAX);
	view_feat.create(1, color_view.rows*color_view.cols, CV_32F);
	for(int r=0; r<view_grad_mag.rows; r++) for(int c=0; c<view_grad_mag.cols; c++)
		view_feat.at<float>(r*view_grad_mag.cols+c) = view_grad_mag.at<float>(r,c);

	return true;
}

bool ObjViewMatcher::MatchView(const Mat& color_view) {
	Mat view_feat;
	ExtractViewFeat(color_view, view_feat);
	BinaryCodes codes;
	HashKey key_value;
	lsh_coder.ComputeCodes(view_feat, codes);
	HashingTools<int>::CodesToKey(codes, key_value);

	// search
	vector<DMatch> matches(obj_db.objects.size());
	for (size_t i=0; i<obj_db.objects.size(); i++) {
		matches[i].distance = (float)norm(view_feat, obj_db.objects[i].visual_desc.img_desc, NORM_L2);	//HashingTools<HashKeyType>::HammingDist(key_value, obj_db.objects[i].visual_desc.key_value);	
		matches[i].trainIdx = i;
	}
	sort(matches.begin(), matches.end(), [](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });

	// visualize
	char str[100];
	for(size_t i=0; i<10; i++) {
		Mat img = imread(obj_db.objects[matches[i].trainIdx].imgpath);
		sprintf_s(str, "%d", i);
		imshow(str, img);
		waitKey(10);
	}

	return true;
}