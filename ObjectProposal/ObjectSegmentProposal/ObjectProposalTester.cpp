#include "ObjectProposalTester.h"

void ObjectProposalTester::TestRankerLearner() {
	processors::attention::ObjectRanker ranker;
	ranker.PrepareRankTrainData(dataset::DB_SALIENCY_RGBD);
	ranker.LearnObjectPredictor();
}

void ObjectProposalTester::Random() {

	visualsearch::processors::attention::ObjectRanker ranker;
	ranker.PrepareRankTrainData(DB_SALIENCY_RGBD);
	return;

	Mat cov_mat, samp_mean;
	Mat feats(2, 3, CV_32F);
	feats.at<float>(0, 0) = 0.9198;
	feats.at<float>(0, 1) = 0.3073;
	feats.at<float>(0, 2) = 0.2049;
	feats.at<float>(1, 0) = 0.6880;
	feats.at<float>(1, 1) = 0.0669;
	feats.at<float>(1, 2) = 0.6891;
	calcCovarMatrix(feats, cov_mat, samp_mean, CV_COVAR_NORMAL | CV_COVAR_ROWS, CV_32F);
	cout<<cov_mat<<endl;

	Eigen::MatrixXd m(cov_mat.rows, cov_mat.cols);
	for(int r=0; r<cov_mat.rows; r++) for(int c=0; c<cov_mat.cols; c++)
		m(r, c) = cov_mat.at<float>(r, c);
	cout<<m<<endl;

	Eigen::EigenSolver<Eigen::MatrixXd> es(m);
	cout<<es.eigenvalues()<<endl;
	cout<<es.eigenvectors()<<endl;

	Mat eigen_values, eigen_vectors;
	eigen(cov_mat, eigen_values, eigen_vectors);	// descending order
	cout<<eigen_vectors<<endl;
	cout<<eigen_values<<endl;
	Mat proj_samps = feats * eigen_vectors.t();
	cout<<proj_samps<<endl;

	return;

	features::Feature3D feat3d;
	Mat cimg = imread(uw_obj_cfn);
	Size newsz;
	ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 50, newsz);
	Mat dmap = imread(uw_obj_dfn, CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	resize(cimg, cimg, newsz);
	resize(dmap, dmap, newsz);
	double minv, maxv;
	minMaxLoc(dmap, &minv, &maxv);
	cout<<minv<<" "<<maxv<<endl;
	imshow("cimg", cimg);
	ImgVisualizer::DrawFloatImg("dmap", dmap);

	Mat res;
	feat3d.ComputeKinect3DMap(dmap, res, true);
	Mat pts_bmap, normal_map, normal_bmap, color_bmap;
	cimg.convertTo(cimg, CV_32F);
	feat3d.ComputeBoundaryMap(cimg, features::BMAP_COLOR, color_bmap);
	feat3d.ComputeBoundaryMap(res, features::BMAP_3DPTS, pts_bmap);
	feat3d.ComputeNormalMap(res, normal_map);
	feat3d.ComputeBoundaryMap(normal_map, features::BMAP_NORMAL, normal_bmap);
	ImgVisualizer::DrawFloatImg("color bmap", color_bmap);
	ImgVisualizer::DrawFloatImg("ptsbmap", pts_bmap);
	ImgVisualizer::DrawNormals("normal", normal_map);
	ImgVisualizer::DrawFloatImg("normalb", normal_bmap);
	RGBDTools rgbd;
	//rgbd.SavePointsToOBJ("clothes.obj", res);
	//io::dataset::Berkeley3DDataManager b3d_man;
	//b3d_man.BrowseData(true, true, true);

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

void ObjectProposalTester::TestSegmentor3D() {

	char str[100];
	for(int id=1; id<180; id++) {
		sprintf_s(str, "meeting_small_1_%d.png", id);
		string cfn = uw_cfn + string(str);
		sprintf_s(str, "meeting_small_1_%d_depth.png", id);
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
		ImgVisualizer::DrawWinsOnImg("gt", cimg, gt_wins);
		waitKey(10);

		imshow("color", cimg);
		ImgVisualizer::DrawFloatImg("dmap", dmap);

		objectproposal::ObjSegmentProposal seg_prop;
		vector<SuperPixel> sps;
		seg_prop.Run(cimg, dmap, 50, sps);

		// only show gt covered objects
		vector<SuperPixel> vis_sps;
		vis_sps.reserve(50);
		vector<SuperPixel> correct_sps;
		for(auto cursp : sps) {
			if(vis_sps.size() < 15)
				vis_sps.push_back(cursp);

			for(size_t i=0; i<gt_wins.size(); i++) {
				Rect intersect_rect = cursp.box & gt_wins[i];
				Rect union_rect = cursp.box | gt_wins[i];
				if( intersect_rect.area()*1.f / union_rect.area() > 0.5f )
					correct_sps.push_back(cursp);
			}
		}

		// display results
		sprintf_s(str, "res_%d.jpg", id);
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
		waitKey(10);
	}
	
}

void ObjectProposalTester::TestBoundaryClf(bool ifTrain) {
	Segmentor3D seg_3d;
	if(ifTrain)
		seg_3d.TrainBoundaryDetector(DB_NYU2_RGBD);
	else {
		Mat cimg = imread(uw_cfn);
		Mat dmap = imread(uw_dfn, CV_LOAD_IMAGE_UNCHANGED);
		dmap.convertTo(dmap, CV_32F);

		imshow("cimg", cimg);
		ImgVisualizer::DrawFloatImg("dmap", dmap);

		seg_3d.RunBoundaryDetection(cimg, dmap, Mat());
	}
}

void ObjectProposalTester::EvaluateOnDataset(DatasetName db_name) {

	RGBDECCV14 rgbd_man;
	FileInfos imgfns, dmapfns;
	rgbd_man.GetImageList(imgfns);
	imgfns.erase(imgfns.begin()+100, imgfns.end());
	rgbd_man.GetDepthmapList(imgfns, dmapfns);
	map<string, vector<Mat>> gt_masks;
	rgbd_man.LoadGTMasks(imgfns, gt_masks);

	vector<Point2f> all_pr;
	float avg_recall = 0;
	for (size_t i=0; i<imgfns.size(); i++) {
		Mat cimg = imread(imgfns[i].filepath);
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
		resize(cimg, cimg, newsz);
		Mat dmap;
		rgbd_man.LoadDepthData(dmapfns[i].filepath, dmap);
		dmap.convertTo(dmap, CV_32F);
		resize(dmap, dmap, newsz);
		vector<Mat>& cur_gt = gt_masks[imgfns[i].filename];
		for(auto& obj : cur_gt) {
			resize(obj, obj, newsz);
		}

		imshow("color", cimg);
		ImgVisualizer::DrawFloatImg("dmap", dmap);
		ImgVisualizer::DrawFloatImg("mask", cur_gt[0]);

		objectproposal::ObjSegmentProposal seg_prop;
		vector<SuperPixel> sps;
		seg_prop.Run(cimg, dmap, 500, sps);

		vector<Point2f> cur_pr;
		seg_prop.ComputePRCurves(sps, cur_gt, 0.7f, cur_pr, true);
		
		// accumulate and compute mean recall
		avg_recall += cur_pr[cur_pr.size()-1].x;

	}

	cout<<"total mean recall top 500: "<<avg_recall / imgfns.size()<<endl;

}

void ObjectProposalTester::TestSegment() {
	visualsearch::processors::segmentation::ImageSegmentor segmentor;
	segmentor.m_dThresholdK = 200;
	Mat cimg = imread(eccv_cfn);
	segmentor.seg_type_ = visualsearch::processors::segmentation::OVER_SEG_GRAPH;
	segmentor.DoSegmentation(cimg);
	imshow("seg", segmentor.m_segImg);
	waitKey(0);

}

void ObjectProposalTester::TestViewMatch() {
	ObjViewMatcher matcher;
	matcher.PrepareDatabase();

	string queryfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-obj-dataset\\rgbd-dataset\\orange\\orange_1\\orange_1_1_1_crop.png";
	Mat view = imread(queryfn);
	matcher.MatchView(view);
}