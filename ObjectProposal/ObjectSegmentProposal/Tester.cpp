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

	ImgVisualizer::DrawFloatImg("dmap", dmap);
	features::Feature3D feat3d;
	Mat pts_3d;
	feat3d.ComputeKinect3DMap(dmap, pts_3d, false);
	//Mat pts_bmap;
	//feat3d.ComputeBoundaryMap(pts_3d, features::BMAP_3DPTS, pts_bmap);
	Mat normal_bmap, normal_map;
	feat3d.ComputeNormalMap(pts_3d, normal_map);
	ImgVisualizer::DrawNormals("normal", normal_map);
	feat3d.ComputeBoundaryMap(normal_map, features::BMAP_NORMAL, normal_bmap);
	imshow("3d", pts_3d);
	ImgVisualizer::DrawFloatImg("bmap", normal_bmap);
	//imwrite("bmap.png", pts_bmap);
	waitKey(0);

	Segmentor3D seg3d;
	seg3d.DIST_TH = 0.03f;
	seg3d.RunRegionGrowing(normal_bmap);

}