#include "Tester.h"

void ObjectProposalTester::TestRankerLearner() {
	processors::attention::ObjectRanker ranker;
	ranker.PrepareRankTrainData(dataset::DB_SALIENCY_RGBD);
	ranker.LearnObjectPredictor();
}

void ObjectProposalTester::BatchProposal() {

	ObjProposalDemo demo;
	visualsearch::io::dataset::NYUDepth2DataMan rgbd;
	FileInfos imgfiles, dmapfiles;
	rgbd.GetImageList(imgfiles);
	rgbd.GetDepthmapList(dmapfiles);
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

void ObjectProposalTester::TestSegmentor3D(const Mat& dmap) {

	ImgVisualizer::DrawFloatImg("", dmap);
	features::Feature3D feat3d;
	Mat pts_3d;
	feat3d.ComputeKinect3DMap(dmap, pts_3d, false);
	imshow("3d", pts_3d);
	waitKey(10);
	vector<vector<FeatPoint>> simg(dmap.rows);
	for(int r=0; r<dmap.rows; r++) {
		simg[r].resize(dmap.cols);
		for(int c=0; c<dmap.cols; c++) {
			simg[r][c].pts_vec = pts_3d.at<Vec3f>(r, c);
		}
	}

	Segmentor3D seg3d;
	seg3d.DIST_TH = 4;
	seg3d.Run(simg);

}