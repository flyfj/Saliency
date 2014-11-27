
#include "ObjectProposalTester.h"

void ObjectProposalTester::TestRankerLearner() {
	processors::attention::ObjectRanker ranker;
	ranker.PrepareRankTrainData(dataset::DB_SALIENCY_RGBD);
	ranker.LearnObjectPredictor();
}

void ObjectProposalTester::Random() {

	//visualsearch::processors::attention::ObjectRanker ranker;
	//ranker.PrepareRankTrainData(DB_SALIENCY_RGBD);
	//return;

	//Mat cov_mat, samp_mean;
	//Mat feats(2, 3, CV_32F);
	//feats.at<float>(0, 0) = 0.9198;
	//feats.at<float>(0, 1) = 0.3073;
	//feats.at<float>(0, 2) = 0.2049;
	//feats.at<float>(1, 0) = 0.6880;
	//feats.at<float>(1, 1) = 0.0669;
	//feats.at<float>(1, 2) = 0.6891;
	//calcCovarMatrix(feats, cov_mat, samp_mean, CV_COVAR_NORMAL | CV_COVAR_ROWS, CV_32F);
	//cout<<cov_mat<<endl;

	//Eigen::MatrixXd m(cov_mat.rows, cov_mat.cols);
	//for(int r=0; r<cov_mat.rows; r++) for(int c=0; c<cov_mat.cols; c++)
	//	m(r, c) = cov_mat.at<float>(r, c);
	//cout<<m<<endl;

	//Eigen::EigenSolver<Eigen::MatrixXd> es(m);
	//cout<<es.eigenvalues()<<endl;
	//cout<<es.eigenvectors()<<endl;

	//Mat eigen_values, eigen_vectors;
	//eigen(cov_mat, eigen_values, eigen_vectors);	// descending order
	//cout<<eigen_vectors<<endl;
	//cout<<eigen_values<<endl;
	//Mat proj_samps = feats * eigen_vectors.t();
	//cout<<proj_samps<<endl;

	//return;

	attention::ObjectRanker ranker;
	ranker.PrepareRankTrainData(DB_NYU2_RGBD);
	

}

void ObjectProposalTester::TestSaliency() {

	Mat cimg = imread(nyu_cfn);
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);
	Mat dmap = imread(nyu_dfn, CV_LOAD_IMAGE_UNCHANGED);
	resize(dmap, dmap, newsz);
	dmap.convertTo(dmap, CV_32F);
	Mat pts3d, normal_map;
	Feature3D feat3d;
	feat3d.ComputeKinect3DMap(dmap, pts3d, true);
	feat3d.ComputeNormalMap(pts3d, normal_map);

	// convert to color image
	Mat dmap_color, pts3d_color, normal_color;
	normalize(dmap, dmap_color, 1, 0, NORM_MINMAX);
	dmap_color.convertTo(dmap_color, CV_8U, 255);
	cvtColor(dmap_color, dmap_color, CV_GRAY2BGR);
	pts3d.convertTo(pts3d_color, CV_8U, 255);
	normal_map.convertTo(normal_color, CV_8U, 255);

	imshow("color", cimg);
	imshow("dmap", dmap_color);
	imshow("pts3d", pts3d_color);
	ImgVisualizer::DrawNormals("normal", normal_map, Mat());

	processors::attention::SaliencyComputer sal_comp;
	Mat cimg_sal, dmap_sal, pts3d_sal, normal_sal;
	sal_comp.ComputeSaliencyMap(cimg, SAL_HC, cimg_sal);
	sal_comp.ComputeSaliencyMap(dmap_color, SAL_HC, dmap_sal);
	sal_comp.ComputeSaliencyMap(pts3d_color, SAL_HC, pts3d_sal);
	sal_comp.ComputeSaliencyMap(normal_color, SAL_HC, normal_sal);
	ImgVisualizer::DrawFloatImg("cimg sal", cimg_sal);
	ImgVisualizer::DrawFloatImg("dmap sal", dmap_sal);
	ImgVisualizer::DrawFloatImg("pts sal", pts3d_sal);
	ImgVisualizer::DrawFloatImg("normal sal", normal_sal);
	cout<<"entropy: "<<endl;
	cout<<"cimg sal: "<<ComputeSaliencyMapEntropy(cimg_sal)<<endl;
	cout<<"dmap sal: "<<ComputeSaliencyMapEntropy(dmap_sal)<<endl;
	cout<<"pts sal: "<<ComputeSaliencyMapEntropy(pts3d_sal)<<endl;
	cout<<"normal sal: "<<ComputeSaliencyMapEntropy(normal_sal)<<endl;

	waitKey(0);
}

void ObjectProposalTester::TestObjSearch() {
	Mat cimg = imread(nyu_cfn);
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);
	Mat dmap = imread(nyu_dfn, CV_LOAD_IMAGE_UNCHANGED);
	resize(dmap, dmap, newsz);

	//ObjectSearcher searcher;
	//searcher.SplitSearch(cimg, dmap);
}

void ObjectProposalTester::TestSuperpixelClf(bool ifTrain) {

	Mat cimg = imread(nyu_cfn);
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);
	imshow("color", cimg);
	Mat dmap = imread(nyu_dfn, CV_LOAD_IMAGE_UNCHANGED);
	resize(dmap, dmap, newsz);
	Mat gtmap = imread(nyu_gtfn, CV_LOAD_IMAGE_UNCHANGED);
	resize(gtmap, gtmap, newsz);
	gtmap.convertTo(gtmap, CV_32S);

	// test semantic merge
	/*IterativeSegmentor iter_seg;
	iter_seg.Init(cimg, dmap);
	iter_seg.Run3();

	return;*/


	//SuperpixelClf sp_clf;
	//sp_clf.Init(SP_COLOR | SP_TEXTURE | SP_NORMAL | SP_DEPTH);

	//if(ifTrain) {
	//	sp_clf.Train(DB_NYU2_RGBD);
	//}
	//else {

	//	sp_clf.Predict(cimg, dmap);
	//	return;

	//	// show superpixel predictions from different images
	//	SegmentProcessor sp_proc;
	//	sp_proc.Init(cimg, dmap);

	//	ImageSegmentor img_seg;
	//	img_seg.m_dThresholdK = 30;
	//	img_seg.m_dMinArea = 100;
	//	img_seg.seg_type_ = OVER_SEG_GRAPH;
	//	img_seg.DoSegmentation(cimg);
	//	imshow("seg", img_seg.m_segImg);

	//	srand(time(NULL));
	//	map<int, vector<int>> sp_gt_ids;
	//	for (size_t i=0; i<img_seg.superPixels.size(); i++) {
	//		sp_proc.ExtractSegmentBasicFeatures(img_seg.superPixels[i]);
	//		int label = gtmap.at<int>(img_seg.superPixels[i].centroid);
	//		sp_gt_ids[label].push_back(i);
	//	}
	//	// classification
	//	sp_clf.Init(SP_COLOR | SP_TEXTURE | SP_NORMAL);
	//	vector<vector<double>> scores(img_seg.superPixels.size());
	//	for(size_t i=0; i<img_seg.superPixels.size(); i++) {
	//		SuperPixel& cur_sp = img_seg.superPixels[i];
	//		Mat feat;
	//		sp_proc.ExtractSegmentVisualFeatures(cur_sp, SP_COLOR | SP_TEXTURE | SP_NORMAL, feat);
	//		//sp_clf.Predict(feat, scores[i]);
	//	}

	//	return;

	//	// compute pair-wise distance
	//	Mat adj_mat;
	//	img_seg.ComputeAdjacencyMat(img_seg.superPixels, adj_mat);
	//	Mat sp_dist(img_seg.superPixels.size(), img_seg.superPixels.size(), CV_32F);
	//	for(int r=0; r<sp_dist.rows; r++) for(int c=r; c<sp_dist.cols; c++) {
	//		sp_dist.at<float>(r,c) = sp_dist.at<float>(c,r) = SuperpixelClf::LabelDistributionDist(scores[r], scores[c]);
	//		/*if(c==r) { sp_dist.at<float>(r, r) = 0; continue; }
	//		if(adj_mat.at<int>(r,c) > 0) {
	//		sp_dist.at<float>(r,c) = sp_dist.at<float>(c,r) = SuperpixelClf::LabelDistributionDist(scores[r], scores[c]);
	//		cout<<"dist: "<<sp_dist.at<float>(r,c)<<endl;
	//		}
	//		else
	//		sp_dist.at<float>(r,c) = sp_dist.at<float>(c,r) = 9999999.f;*/
	//	}

	//	Mat geo_dist;
	//	//PointSegmentor::ComputeFloydWarshall(sp_dist, geo_dist);

	//	while(1) {
	//		int sp_id = rand() % img_seg.superPixels.size();
	//		vector<SuperPixel> sps;
	//		sps.push_back(img_seg.superPixels[sp_id]);
	//		ImgVisualizer::DrawShapes(cimg, sps, Mat(), false);
	//		Mat obj_map(cimg.rows, cimg.cols, CV_32F);
	//		obj_map.setTo(0);

	//		for (size_t i=0; i<img_seg.superPixels.size(); i++) {
	//			obj_map.setTo(sp_dist.at<float>(sp_id, i), img_seg.superPixels[i].mask);
	//		}
	//		normalize(obj_map, obj_map, 1, 0, NORM_MINMAX);
	//		obj_map = 1 - obj_map;
	//		ImgVisualizer::DrawFloatImg("dist map", obj_map);
	//		if( waitKey(0) == 'q' )
	//			break;
	//	}
	//	
	//	//int sel_label = 19;
	//	//ofstream out("sp_pred_19.txt");
	//	//sp_clf.Init(SP_COLOR | SP_TEXTURE | SP_NORMAL);
	//	//for(size_t i=0; i<sp_gt_ids[sel_label].size(); i++) {
	//	//	int seg_id = sp_gt_ids[sel_label][i];
	//	//	SuperPixel& cur_sp = img_seg.superPixels[seg_id];
	//	//	imshow("mask1", cur_sp.mask*255);
	//	//	vector<double> scores;
	//	//	
	//	//	Mat feat;
	//	//	sp_proc.ExtractSegmentVisualFeatures(cur_sp, SP_COLOR | SP_TEXTURE | SP_NORMAL, feat);
	//	//	sp_clf.Predict(feat, scores);
	//	//	// output results
	//	//	for(size_t i=0; i<scores.size(); i++) {
	//	//		out<<scores[i]<<" ";
	//	//	}
	//	//	out<<endl;
	//	//}
	//	
	//}
}

float ObjectProposalTester::ComputeSaliencyMapEntropy(const Mat& sal_map) {
	int bin_num = 20;
	float step = 1.f / bin_num;
	learners::trees::NodeStatisticsHist hist(bin_num);
	for (int r=0; r<sal_map.rows; r++) for(int c=0; c<sal_map.cols; c++) {
		int bin_id = MIN(int(sal_map.at<float>(r,c)/step), bin_num-1);
		hist.AddSample(bin_id, 1);
	}

	return hist.Entropy();
}

void ObjectProposalTester::BoundaryPlayground() {
	
	vector<Mat> all_imgs(10);
	Mat cimg = imread(nyu_cfn);
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);
	cimg.copyTo(all_imgs[0]);
	cvtColor(cimg, cimg, CV_BGR2Lab);
	Mat lab_cimg;
	cimg.convertTo(lab_cimg, CV_32F, 1.f/255);

	Mat dmap = imread(nyu_dfn, CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	resize(dmap, dmap, newsz);
	
	visualsearch::features::Feature3D feat3d;
	Mat color_bmap, pts3d, pts_bmap, normal_map, normal_bmap, tbmap;
	feat3d.ComputeKinect3DMap(dmap, pts3d, false);
	feat3d.ComputeBoundaryMap(lab_cimg, pts3d, Mat(), features::BMAP_COLOR, color_bmap);
	RGBDTools tool;
	//tool.SavePointsToOBJ("scene.obj", pts3d);
	//return;
	feat3d.ComputeBoundaryMap(Mat(), pts3d, Mat(), features::BMAP_3DPTS, pts_bmap);
	feat3d.ComputeNormalMap(pts3d, normal_map);
	feat3d.ComputeBoundaryMap(Mat(), Mat(), normal_map, features::BMAP_NORMAL, normal_bmap);
	feat3d.ComputeBoundaryMap(lab_cimg, pts3d, normal_map, BMAP_3DPTS, tbmap);
	double minv, maxv;
	minMaxLoc(color_bmap, &minv, &maxv);
	cout<<minv<<" "<<maxv<<endl;
	ImgVisualizer::DrawFloatImg("combined bmap", tbmap, all_imgs[7], false);

	tbmap.convertTo(tbmap, CV_8U, 255);
	color_bmap.convertTo(color_bmap, CV_8U, 255);
	normal_bmap.convertTo(normal_bmap, CV_8U, 255);
	imwrite("color.png", color_bmap);
	imwrite("normal.png", normal_bmap);
	imwrite("pts.png", tbmap);

	cvtColor(tbmap, tbmap, CV_GRAY2BGR);
	
	double start_t = GetTickCount();
	visualsearch::processors::segmentation::ImageSegmentor segmentor;
	segmentor.m_dThresholdK = 1000;
	segmentor.m_dMinArea = 200;
	segmentor.seg_type_ = visualsearch::processors::segmentation::OVER_SEG_GRAPH;
	segmentor.slic_seg_num_ = 2;
	int num = segmentor.DoSegmentation(tbmap);
	cout<<"segment number: "<<num<<endl;
	cout<<"segment time: "<<(GetTickCount()-start_t) / getTickFrequency()<<"s."<<endl;
	segmentation::SegmentProcessor seg_proc;
	for(size_t i=0; i<segmentor.superPixels.size(); i++) {
		seg_proc.ExtractSegmentBasicFeatures(segmentor.superPixels[i]);
	}
	all_imgs[8] = segmentor.m_segImg;

	Mat combine_bmap;
	Mat adj_mat;
	segmentor.ComputeAdjacencyMat(segmentor.superPixels, adj_mat);
	feat3d.ComputeBoundaryMapWithSuperpixels(lab_cimg, pts3d, normal_map, BMAP_3DPTS | BMAP_COLOR, segmentor.superPixels, adj_mat, combine_bmap);

	// show
	ImgVisualizer::DrawFloatImg("depth", dmap, all_imgs[1], false);
	ImgVisualizer::DrawFloatImg("color bmap", color_bmap, all_imgs[4], false);
	ImgVisualizer::DrawFloatImg("pts3d", pts3d, all_imgs[2], false);
	ImgVisualizer::DrawFloatImg("3d bmap", pts_bmap, all_imgs[5], false);
	ImgVisualizer::DrawNormals("normal", normal_map, all_imgs[3]);
	ImgVisualizer::DrawFloatImg("normal bmap", normal_bmap, all_imgs[6], false);
	ImgVisualizer::DrawFloatImg("combine sp bmap", combine_bmap, all_imgs[9], false);
	// combine
	ImgVisualizer::DrawImgCollection("all", all_imgs, 5, Size(50,50), Mat());


	waitKey(10);
}

void ObjectProposalTester::BatchProposal() {

	ObjProposalDemo demo;
	visualsearch::io::dataset::NYUDepth2DataMan rgbd;
	FileInfos imgfiles, dmapfiles;
	rgbd.GetImageList(imgfiles);
	rgbd.GetDepthmapList(imgfiles, dmapfiles);
	imgfiles.erase(imgfiles.begin(), imgfiles.begin()+800);
	dmapfiles.erase(dmapfiles.begin(), dmapfiles.begin()+800);

	string savedir = "E:\\res\\segments\\";
	char str[100];
	for(size_t i=0; i<imgfiles.size(); i++)
	{
		Mat cimg = imread(imgfiles[i].filepath);
		Size newsz;
		tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
		resize(cimg, cimg, newsz);
		Mat dmap;
		rgbd.LoadDepthData(dmapfiles[i].filepath, dmap);
		resize(dmap, dmap, newsz);

		imshow("color", cimg);
		ImgVisualizer::DrawFloatImg("dmap", dmap, Mat());

		Mat oimg;
		demo.RunObjSegProposal(cimg, dmap, oimg);
		newsz.width = 600;
		newsz.height = newsz.width * oimg.rows / oimg.cols;
		resize(oimg, oimg, newsz);
		imshow("res", oimg);
		waitKey(10);

		string savefn = savedir + "nyu_" + imgfiles[i].filename + "_res.png";
		imwrite(savefn, oimg);
	}

}

void ObjectProposalTester::ProposalDemo() {
	ObjProposalDemo demo;
	demo.RunVideoDemo(SENSOR_CAMERA, DEMO_OBJECT_SEG);

}

void ObjectProposalTester::TestSegmentor3D() {

	char str[100];
	vector<vector<Point2f>> all_pr;
	int common_num = 0;
	float avg_recall = 0;
	for(int id=50; id<100; id++) {
		sprintf_s(str, "table_small_1_%d.png", id);
		string cfn = uw_cfn + string(str);
		sprintf_s(str, "table_small_1_%d_depth.png", id);
		string dfn = uw_dfn + string(str);
		sprintf_s(str, "%d.txt", id);
		string gtfn = uw_gt_dir + string(str);

		// process
		Mat cimg = imread(cfn);
		Mat dmap = imread(dfn, CV_LOAD_IMAGE_UNCHANGED);
		dmap.convertTo(dmap, CV_32F);
		Size newsz;
		float ratio = ToolFactory::compute_downsample_ratio(Size(dmap.cols, dmap.rows), 400, newsz);
		resize(cimg, cimg, newsz);
		resize(dmap, dmap, newsz);
		// load gt boxes
		ifstream in(gtfn.c_str());
		int win_num;
		in>>win_num;
		if(win_num == 0) continue;
		vector<ImgWin> gt_wins(win_num);
		for(int i=0; i<win_num; i++) {
			int xmin, ymin, xmax, ymax;
			in>>xmin>>ymin>>xmax>>ymax;
			xmin = (int)(xmin*ratio);
			ymin = (int)(ymin*ratio);
			xmax = (int)(xmax*ratio);
			ymax = (int)(ymax*ratio);
			gt_wins[i] = ImgWin(xmin, ymin, xmax-xmin, ymax-ymin);
		}
		vector<Mat> gt_masks(win_num);
		for(size_t i=0; i<gt_wins.size(); i++) {
			gt_masks[i].create(cimg.rows, cimg.cols, CV_8U);
			gt_masks[i].setTo(0);
			gt_masks[i](gt_wins[i]).setTo(1);
		}
		ImgVisualizer::DrawWinsOnImg("gt", cimg, gt_wins);
		waitKey(10);

		imshow("color", cimg);
		ImgVisualizer::DrawFloatImg("dmap", dmap);

		objectproposal::ObjSegmentProposal seg_prop;
		vector<VisualObject> sps;
		seg_prop.Run(cimg, dmap, -1, sps);

		vector<Point2f> cur_pr;
		//seg_prop.ComputePRCurves(sps, gt_masks, 0.6f, cur_pr, false);
		// accumulate and compute mean recall
		avg_recall += cur_pr[cur_pr.size()-1].x;

		all_pr.push_back(cur_pr);
		if(cur_pr.size() > common_num) common_num = cur_pr.size();

		cout<<"finish image "<<id<<"/"<<cfn<<endl;

		//// only show gt covered objects
		//vector<SuperPixel> vis_sps;
		//vis_sps.reserve(50);
		//vector<SuperPixel> correct_sps;
		//for(auto cursp : sps) {
		//	if(vis_sps.size() < 15)
		//		vis_sps.push_back(cursp);

		//	for(size_t i=0; i<gt_wins.size(); i++) {
		//		Rect intersect_rect = cursp.box & gt_wins[i];
		//		Rect union_rect = cursp.box | gt_wins[i];
		//		if( intersect_rect.area()*1.f / union_rect.area() > 0.5f )
		//			correct_sps.push_back(cursp);
		//	}
		//}

		// display results
		/*sprintf_s(str, "res_%d.jpg", id);
		mkdir(save_dir.c_str());
		string savefn = save_dir + string(str);
		Mat oimg;
		ImgVisualizer::tmp_draw_gt = false;
		ImgVisualizer::DrawShapes(cimg, vis_sps, oimg, false);
		ImgVisualizer::tmp_draw_gt = true;
		ImgVisualizer::DrawShapes(oimg, correct_sps, oimg, false);
		resize(oimg, oimg, Size(oimg.cols*2, oimg.rows*2));
		imwrite(savefn, oimg);
		imshow("results", oimg);
		waitKey(10);*/
	}
	// get mean pr
	vector<Point2f> mean_pr(common_num);
	for(size_t j=0; j<common_num; j++) {
		for(size_t i=0; i<all_pr.size(); i++) {
			if(j > all_pr[i].size()) mean_pr[j] += all_pr[i][all_pr[i].size()-1];
			else mean_pr[j] += all_pr[i][j];
		}
		mean_pr[j].x /= all_pr.size();
		mean_pr[j].y /= all_pr.size();
	}

	ofstream out("uw_all_pr.txt");
	for(size_t i=0; i<mean_pr.size(); i++) {
		out<<mean_pr[i].x<<" "<<mean_pr[i].y<<endl;
	}
	cout<<"total mean recall: "<<avg_recall / 50<<endl;
	
}

void ObjectProposalTester::TestBoundaryClf(bool ifTrain) {
	/*Segmentor3D seg_3d;
	if(ifTrain)
	seg_3d.TrainBoundaryDetector(DB_NYU2_RGBD);
	else {
	Mat cimg = imread(nyu_cfn);
	Mat dmap = imread(nyu_dfn, CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);

	imshow("cimg", cimg);
	ImgVisualizer::DrawFloatImg("dmap", dmap);

	seg_3d.RunBoundaryDetection(cimg, dmap, Mat());
	}*/
}

bool ObjectProposalTester::LoadNYU20Masks(FileInfo imgfn, vector<Mat>& gt_masks) {

	string img_fn_no_ext = imgfn.filename.substr(0, 5);
	char str[50];
	sprintf_s(str, "%s-mask*.png", img_fn_no_ext.c_str());
	FileInfos maskfns;
	ToolFactory::GetFilesFromDir(nyu20_gtdir, str, maskfns);
	gt_masks.reserve(maskfns.size());
	for(size_t i=0; i<maskfns.size(); i++) {
		Mat cur_mask = imread(maskfns[i].filepath, CV_LOAD_IMAGE_GRAYSCALE);
		if(countNonZero(cur_mask)*1.f < cur_mask.rows*cur_mask.cols*0.005) continue;
		gt_masks.push_back(cur_mask);
	}

	return true;
}

#define NYU20
//#define UW_SCENE
void ObjectProposalTester::EvaluateOnDataset(DatasetName db_name) {

	DataManagerInterface* db_man = NULL;
	if(db_name == DB_SALIENCY_RGBD)
		db_man = new RGBDECCV14;
	if(db_name == DB_NYU2_RGBD)
		db_man = new NYUDepth2DataMan;
	if(db_name == DB_BERKELEY3D)
		db_man = new Berkeley3DDataManager;

	srand(time(0));
	FileInfos imgfns, dmapfns;
	db_man->GetImageList(imgfns);
	random_shuffle(imgfns.begin(), imgfns.end());
	imgfns.erase(imgfns.begin()+1, imgfns.end());
	//imgfns[0].filepath = eccv_cfn + "11_03-46-20.jpg";
	//imgfns[0].filename = "11_03-46-20.jpg";
	//imgfns.erase(imgfns.begin()+10, imgfns.end());
	//b3d_man.GetDepthmapList(imgfns, dmapfns);
	db_man->GetDepthmapList(imgfns, dmapfns);
	map<string, vector<Mat>> gt_masks;
	map<string, vector<ImgWin>> gt_boxes;
	//b3d_man.LoadGTWins(imgfns, gt_boxes);

#ifdef NYU20

	ToolFactory::GetFilesFromDir(nyu20_cdir, "*.png", imgfns);
	//imgfns.erase(imgfns.begin(), imgfns.begin()+9);
	//imgfns.erase(imgfns.begin()+10, imgfns.end());
	dmapfns.resize(imgfns.size());
	for(size_t i=0; i<imgfns.size(); i++) {
		string img_fn_no_ext = imgfns[i].filename.substr(0, 5);
		dmapfns[i].filename = img_fn_no_ext + "-depth.png";
		dmapfns[i].filepath = nyu20_ddir + dmapfns[i].filename;
	}

#endif

#ifdef UW_SCENE

	imgfns.clear();
	dmapfns.clear();
	char str[50];
	for(int id=50; id<60; id++) {
		sprintf_s(str, "table_small_1_%d.png", id);
		string cfn = uw_cfn + string(str);
		FileInfo imgfn;
		imgfn.filename = string(str);
		imgfn.filepath = cfn;
		imgfns.push_back(imgfn);
		sprintf_s(str, "table_small_1_%d_depth.png", id);
		string dfn = uw_dfn + string(str);
		FileInfo dmapfn;
		dmapfn.filename = string(str);
		dmapfn.filepath = dfn;
		dmapfns.push_back(dmapfn);
		sprintf_s(str, "%d.txt", id);
		string gtfn = uw_gt_dir + string(str);
		// load gt boxes
		ifstream in(gtfn.c_str());
		int win_num;
		in>>win_num;
		if(win_num == 0) continue;
		vector<ImgWin> gt_wins(win_num);
		for(int i=0; i<win_num; i++) {
			int xmin, ymin, xmax, ymax;
			in>>xmin>>ymin>>xmax>>ymax;
			xmin = (int)(xmin*ratio);
			ymin = (int)(ymin*ratio);
			xmax = (int)(xmax*ratio);
			ymax = (int)(ymax*ratio);
			gt_wins[i] = ImgWin(xmin, ymin, xmax-xmin, ymax-ymin);
		}
		vector<Mat> gts(win_num);
		for(size_t i=0; i<gt_wins.size(); i++) {
			gts[i].create(cimg.rows, cimg.cols, CV_8U);
			gts[i].setTo(0);
			gts[i](gt_wins[i]).setTo(1);
		}
		gt_masks[imgfn.filename] = gts;
	}
	

#endif

	_mkdir(save_dir.c_str());

	vector<vector<Point2f>> all_prs_05(imgfns.size());
	vector<vector<Point2f>> all_prs_06(imgfns.size());
	vector<vector<Point2f>> all_prs_07(imgfns.size());
	float avg_recall_05 = 0;
	float avg_recall_06 = 0;
	float avg_recall_07 = 0;
	vector<Point3f> best_gt_cover;
	int common_num = 0;
	int valid_img_num = 0;
//#pragma omp parallel for
	for (int i=0; i<imgfns.size(); i++) {

		cout<<"Processing "<<i<<"/"<<imgfns.size()<<endl;
		Mat cimg = imread(imgfns[i].filepath);
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
		resize(cimg, cimg, newsz);
		
		Mat dmap;
#if defined(NYU20) || defined(UW_SCENE)
		dmap = imread(dmapfns[i].filepath, CV_LOAD_IMAGE_UNCHANGED);
#else
		db_man->LoadDepthData(dmapfns[i].filepath, dmap);
#endif
		dmap.convertTo(dmap, CV_32F);
		resize(dmap, dmap, newsz);

#ifdef NYU20
		LoadNYU20Masks(imgfns[i], gt_masks[imgfns[i].filename]);
#else
		FileInfos cur_fns;
		cur_fns.push_back(imgfns[i]);
		db_man->LoadGTMasks(cur_fns, gt_masks);
#endif
		vector<Mat>& cur_gts = gt_masks[imgfns[i].filename];
		if(cur_gts.empty()) continue;
		valid_img_num++;

		for(auto& obj : cur_gts) {
			resize(obj, obj, newsz);
		}

		//imshow("color", cimg);
		//ImgVisualizer::DrawFloatImg("dmap", dmap);
		//ImgVisualizer::DrawFloatImg("mask", cur_gts[0]);

		objectproposal::ObjSegmentProposal seg_prop;
		vector<VisualObject> sps;
		double start_t = getTickCount();
		seg_prop.Run(cimg, dmap, -1, sps);
		cout<<"Process time: "<<(getTickCount()-start_t)/getTickFrequency()<<"s."<<endl;
		//seg_prop.GetCandidatesFromIterativeSeg(cimg, dmap, sps);

#define SAVE
#ifdef SAVE
		// save proposal to folder
		for(size_t id=0; id<sps.size(); id++) {
			char str[30];
			sprintf_s(str, "_%d.png", id);
			string save_sub_dir = save_dir + imgfns[i].filename + "\\";
			_mkdir(save_sub_dir.c_str());
			string savefn = save_sub_dir + string(str);
			imwrite(savefn, sps[id].visual_data.mask*255);
		}

		//continue;
#endif

		// evaluation
		vector<Point2f> cur_pr;
		vector<Point3f> best_overlap;
		seg_prop.ComputePRCurves(sps, cur_gts, 0.6f, cur_pr, best_overlap, true);

		// save best results
		for(size_t k=0; k<best_overlap.size(); k++) {
			vector<VisualObject> show_sps;
			show_sps.push_back(sps[best_overlap[k].y]);
			Mat resimg;
			ImgVisualizer::DrawShapes(cimg, show_sps, resimg, false);
			//imshow("res", resimg);
			//waitKey(0);
			char str[30];
			sprintf_s(str, "nyu_best_%d_%d.png", i, k);
			string savefn = save_dir + str;
			imwrite(savefn, resimg);
		}
		
		best_gt_cover.insert(best_gt_cover.end(), best_overlap.begin(), best_overlap.end());
		all_prs_06[i] = cur_pr;
		avg_recall_06 += cur_pr[cur_pr.size()-1].x;
		if(cur_pr.size() > common_num) common_num = cur_pr.size();
		cout<<"mean recall: "<<avg_recall_06 / valid_img_num<<endl;
		// ABO
		float abo = 0;
		for(auto curval : best_gt_cover) { abo += curval.z;}
		cout<<"ABO: "<<abo/best_gt_cover.size()<<endl;

		cout<<"finish image "<<i<<"/"<<imgfns.size()<<endl<<endl;
		waitKey(10);
	}

	delete db_man;
	db_man = NULL;
	
	vector<Point2f> mean_pr(common_num);
	for(size_t j=0; j<common_num; j++) {
		mean_pr[j] = Point2f(0, 0);
		for(size_t i=0; i<all_prs_06.size(); i++) {
			if(j >= all_prs_06[i].size()) 
				mean_pr[j] += all_prs_06[i][all_prs_06[i].size()-1];
			else 
				mean_pr[j] += all_prs_06[i][j];
		}
		mean_pr[j].x /= all_prs_06.size();
		mean_pr[j].y /= all_prs_06.size();
		if(mean_pr[j].x < 0)
			cout<<"error"<<endl;
	}
	
	ofstream out("nyu20_tmp_pr.txt");
	for(size_t i=0; i<mean_pr.size(); i++) {
		out<<mean_pr[i].x<<" "<<mean_pr[i].y<<endl;
	}
	cout<<"total mean recall: "<<avg_recall_06 / valid_img_num<<endl;
}

void ObjectProposalTester::TestSegment() {
	Mat cimg = imread(nyu_cfn);
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
	cout<<newsz.width<<" "<<newsz.height<<endl;
	resize(cimg, cimg, newsz);
	Mat dmap = imread(nyu_dfn, CV_LOAD_IMAGE_UNCHANGED);
	resize(dmap, dmap, newsz);
	segmentation::IterativeSegmentor iter_segmentor;
	iter_segmentor.Init(cimg, dmap);
	iter_segmentor.Run2();
	return;

	visualsearch::processors::segmentation::ImageSegmentor segmentor;
	segmentor.m_dThresholdK = 25;
	imshow("color", cimg);
	segmentor.seg_type_ = visualsearch::processors::segmentation::OVER_SEG_GRAPH;
	segmentor.DoSegmentation(cimg);
	imshow("seg", segmentor.m_segImg);
	waitKey(0);

}

void ObjectProposalTester::Build3DPCL(DatasetName db_name) {

	DataManagerInterface* db_man = NULL;
	if(db_name == DB_SALIENCY_RGBD)
		db_man = new RGBDECCV14;
	if(db_name == DB_NYU2_RGBD)
		db_man = new NYUDepth2DataMan;
	
	FileInfos imgfns, dmapfns;
	db_man->GetImageList(imgfns);
	imgfns.erase(imgfns.begin()+10, imgfns.end());
	db_man->GetDepthmapList(imgfns, dmapfns);

	char str[100];
	for(size_t i=0; i<imgfns.size(); i++) {

		Mat dmap;
		db_man->LoadDepthData(dmapfns[i].filepath, dmap);

		RGBDTools rgbd;
		Mat pts3d;
		rgbd.KinectDepthTo3D(dmap, pts3d);
		sprintf_s(str, "%s.obj", imgfns[i].filename.c_str());
		string savefn = save_dir + str;
		rgbd.SavePointsToOBJ(savefn, pts3d);
	}

}

void ObjectProposalTester::TestViewMatch() {
	//ObjViewMatcher matcher;
	//matcher.PrepareDatabase();

	//Mat cimg = imread(uw_cfn + "table_small_1_45.png");
	//Size newsz;
	//ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
	//resize(cimg, cimg, newsz);
	////Mat dmap = imread(uw_dfn, CV_LOAD_IMAGE_UNCHANGED);

	//matcher.SearchImage(cimg, Mat());

	/*string queryfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\orange\\orange_1\\orange_1_1_1_crop.png";
	Mat view = imread(queryfn);
	resize(view, view, Size(25,25));
	imshow("query", view);
	matcher.MatchView(view, Mat());*/
}

void ObjectProposalTester::TestPatchMatcher() {

	//Mat cimg = imread(uw_cfn + "table_small_1_45.png");
	Mat cimg = imread(nyu_cfn);
	Size newsz(400, 400);
	//ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);
	Mat dmap = imread(nyu_dfn, CV_LOAD_IMAGE_UNCHANGED);
	resize(dmap, dmap, newsz);
	imshow("color", cimg);
	ImgVisualizer::DrawFloatImg("dmap", dmap);

	//Mat scoremap;
	//ObjPatchMatcher pmatcher;
	//pmatcher.use_depth = true;
	//pmatcher.use_code = false;
	//pmatcher.patch_size = Size(11, 11);
	////pmatcher.PrepareViewPatchDB();
	////pmatcher.MatchViewPatch(cimg, Mat());
	//pmatcher.PreparePatchDB(DB_NYU2_RGBD);
	//pmatcher.Match(cimg, dmap, scoremap);
	//ImgVisualizer::DrawFloatImg("scoremap", scoremap);
	//imshow("cimg", cimg);
	//waitKey(0);

	//return;

	//// do segmentation
	//float ths[] = {0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f};
	//for(size_t i=0; i<8; i++) {
	//	Mat th_mask;
	//	threshold(scoremap, th_mask, ths[i], 1, CV_THRESH_BINARY);
	//	th_mask.convertTo(th_mask, CV_8U);
	//	imshow("th mask", th_mask*255);
	//	waitKey(0);
	//	if(countNonZero(th_mask) == 0) {
	//		cerr<<"no fg points found."<<endl;
	//		continue;
	//	}

	//	ImageSegmentor img_segmentor;
	//	img_segmentor.m_dMinArea = 30;
	//	img_segmentor.m_dThresholdK = 30;
	//	img_segmentor.seg_type_ = OVER_SEG_GRAPH;
	//	img_segmentor.DoSegmentation(cimg);
	//	Mat markers(scoremap.size(), CV_32S);
	//	markers = Scalar::all(0);
	//	int cnt = 1;
	//	vector<bool> valid_sps(img_segmentor.superPixels.size(), false);
	//	// take superpixel with contour points as initial region
	//	for(int r=0; r<th_mask.rows; r++) for(int c=0; c<th_mask.cols; c++) {
	//		if(th_mask.at<uchar>(r,c) > 0) {
	//			int cur_id = img_segmentor.m_idxImg.at<int>(r,c);
	//			if(!valid_sps[cur_id]) {
	//				markers.setTo(cnt, img_segmentor.superPixels[cur_id].mask);
	//				valid_sps[cur_id] = true;
	//				cnt++;
	//			}
	//		}
	//	}
	//	ImgVisualizer::DrawFloatImg("markers", markers);
	//	waitKey(0);

	//	vector<SuperPixel> sps;
	//	RunWatershed(cimg, markers, cnt, sps);
	//	for(size_t j=0; j<sps.size(); j++) {
	//		Mat disp_img = Mat::zeros(cimg.rows, cimg.cols, CV_8UC3);
	//		imshow("sp mask", sps[j].mask*255);
	//		cimg.copyTo(disp_img, sps[j].mask);
	//		imshow("disp", disp_img);
	//		waitKey(0);
	//	}
	//}
}

void ObjectProposalTester::TestGraphcut() {

	Mat cimg = imread(eccv_cfn);
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
	resize(cimg, cimg, newsz);

	ImageSegmentor img_segmentor;
	img_segmentor.m_dMinArea = 30;
	img_segmentor.m_dThresholdK = 30;
	img_segmentor.seg_type_ = OVER_SEG_GRAPH;
	img_segmentor.DoSegmentation(cimg);
	imshow("seg", img_segmentor.m_segImg);

	srand(time(0));
	vector<VisualObject>& sps = img_segmentor.superPixels;
	while(1) {
		random_shuffle(sps.begin(), sps.end());
		Mat fg_mask(cimg.rows, cimg.cols, CV_8U);
		fg_mask.setTo(GC_PR_FGD);
		fg_mask.setTo(GC_PR_BGD, sps[1].visual_data.mask);
		fg_mask.setTo(GC_FGD, sps[0].visual_data.mask);
		for(size_t i=0; i<sps.size(); i++) {
			if(sps[i].visual_data.is_boundary) fg_mask.setTo(GC_BGD, sps[i].visual_data.mask);
		}
		ImgVisualizer::DrawFloatImg("init mask", fg_mask);
		waitKey(0);

		Mat fgmodel, bgmodel;
		grabCut(cimg, fg_mask, Rect(0,0,1,1), bgmodel, fgmodel, 3, GC_INIT_WITH_MASK);
		fg_mask &= 1;
		Mat res_img;
		cimg.copyTo(res_img, fg_mask);
		imshow("fg", res_img);
		if( waitKey(0) == 'q' )
			break;
	}
	
	

}

void ObjectProposalTester::RunWatershed(const Mat& cimg, Mat& markers, int region_num, vector<VisualObject>& sps) {
	
	// markers contains positive labels for each superpixel
	if(markers.depth() != CV_32S) return;
	watershed(cimg, markers);

	sps.clear();
	sps.resize(region_num);
	for(size_t i=0; i<sps.size(); i++) {
		sps[i].visual_data.mask = Mat::zeros(cimg.rows, cimg.cols, CV_8U);
	}
	for(int r=0; r<markers.rows; r++) {
		for(int c=0; c<markers.cols; c++) {
			if(markers.at<int>(r,c) == -1) continue;
			sps[markers.at<int>(r,c)-1].visual_data.mask.at<uchar>(r,c) = 1;
		}
	}

}