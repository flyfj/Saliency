// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//
#include "Features/Feature3D.h"
#include "Tester.h"
#include "Processors/Segmentation/FixationSegmentator.h"

#include "ObjSegmentProposal.h"
#include "IO/Dataset/NYUDepth2DataMan.h"
#include "ObjectRanker.h"
#include "ObjProposalDemo.h"
#include "Common/Tools/RGBDTools.h"
#include "Processors/Segmentation/IterativeSegmentor.h"


// link
#ifdef _DEBUG
//#pragma comment(lib, "pcl_common_debug.lib")
//#pragma comment(lib, "pcl_features_debug.lib")
//#pragma comment(lib, "pcl_filters_debug.lib")
//#pragma comment(lib, "pcl_visualization_debug.lib")
//#pragma comment(lib, "pcl_search_debug.lib")
//#pragma comment(lib, "pcl_kdtree_debug.lib")
//#pragma comment(lib, "pcl_keypoints_debug.lib")
//#pragma comment(lib, "pcl_registration_debug.lib")
//#pragma comment(lib, "pcl_io_debug.lib")
//#pragma comment(lib, "pcl_ml_debug.lib")
//#pragma comment(lib, "pcl_recognition_debug.lib")
#pragma comment(lib, "opencv_core249d.lib")
#pragma comment(lib, "opencv_imgproc249d.lib")
#pragma comment(lib, "opencv_highgui249d.lib")
#pragma comment(lib, "opencv_ml249d.lib")
#pragma comment(lib, "opencv_features2d249d.lib")
#pragma comment(lib, "opencv_nonfree249d.lib")
#pragma comment(lib, "opencv_objdetect249d.lib")
#pragma comment(lib, "opencv_flann249d.lib")

#else
#pragma comment(lib, "pcl_common_release.lib")
#pragma comment(lib, "pcl_features_release.lib")
#pragma comment(lib, "pcl_filters_release.lib")
#pragma comment(lib, "pcl_visualization_release.lib")
#pragma comment(lib, "pcl_search_release.lib")
#pragma comment(lib, "pcl_kdtree_release.lib")
#pragma comment(lib, "pcl_keypoints_release.lib")
#pragma comment(lib, "pcl_registration_release.lib")
#pragma comment(lib, "pcl_io_release.lib")
#pragma comment(lib, "pcl_ml_release.lib")
#pragma comment(lib, "pcl_recognition_release.lib")
#pragma comment(lib, "opencv_core249.lib")
#pragma comment(lib, "opencv_imgproc249.lib")
#pragma comment(lib, "opencv_highgui249.lib")
#pragma comment(lib, "opencv_ml249.lib")
#pragma comment(lib, "opencv_features2d249.lib")
#pragma comment(lib, "opencv_nonfree249.lib")
#pragma comment(lib, "opencv_objdetect249.lib")
#pragma comment(lib, "opencv_flann249.lib")
#endif

//#pragma comment(lib, "vtkCommonCore-6.1.lib")
//#pragma comment(lib, "vtkCommonDataModel-6.1.lib")
//#pragma comment(lib, "vtkCommonMath-6.1.lib")
//#pragma comment(lib, "vtkCommonColor-6.1.lib")
//#pragma comment(lib, "vtkRenderingCore-6.1.lib")
//#pragma comment(lib, "vtkRenderingLOD-6.1.lib")
//#pragma comment(lib, "vtkFiltersSources-6.1.lib")
//#pragma comment(lib, "vtkCommonExecutionModel-6.1.lib")



int main()
{
	/*ObjectProposalTester tester;
	tester.TestRankerLearner();
	getchar();
	return 0;*/

	Mat dmap, cimg, mat3dpts, normals;
	visualsearch::common::tools::RGBDTools rgbdtool;
	objectproposal::ObjSegmentProposal prop;
	visualsearch::processors::segmentation::IterativeSegmentor segmentor;
	visualsearch::processors::segmentation::ImageSegmentor img_segmentor;
	ObjProposalDemo demo;
	segmentation::FixationSegmentor fix_seg;
	//demo.RunVideoDemo(SENSOR_KINECT, DEMO_VIEW_ONLY);
	//return 0;
	ObjectProposalTester tester;
	//tester.BatchProposal();

	//return 0;
	string nyu_cfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\211.jpg";
	string nyu_dfn = "E:\\Datasets\\RGBD_Dataset\\NYU\\Depth2\\211_d.png";
	string uw_cfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scenes-v2_imgs\\imgs\\scene_02\\00001-color.png";
	string uw_dfn = "E:\\Datasets\\RGBD_Dataset\\UW\\rgbd-scenes-v2_imgs\\imgs\\scene_02\\00001-depth.png";
	string eccv_cfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\RGB\\8_08-34-01.jpg";
	string eccv_dfn = "E:\\Datasets\\RGBD_Dataset\\Saliency\\Depth\\smoothedDepth\\8_08-34-01_Depth.png";
	cimg = imread(eccv_cfn);
	imshow("cimg", cimg);
	dmap = imread(eccv_dfn, CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	tester.TestSegmentor3D(dmap);
	visualsearch::features::Feature3D feat3d;
	Mat feat_map, normal_mat;
	feat3d.ComputeKinect3DMap(dmap, feat_map);
	feat3d.ComputeNormalMap(feat_map, normal_mat);
	feat3d.ComputeBoundaryMap(feat_map, features::BMAP_3DPTS, normal_mat);
	ImgVisualizer::DrawFloatImg("bmap", feat_map);
	waitKey(0);
	ImgVisualizer::DrawNormals("normal", normal_mat);
	//fix_seg.Init(cimg, dmap, segmentation::SP_COLOR);
	//fix_seg.ExtractObjects(250, 200, vector<SuperPixel>());
	//resize(dmap, dmap, Size(dmap.cols/2, dmap.rows/2));
	//rgbdtool.KinectDepthTo3D(dmap, mat3dpts);
	//mat3dpts.convertTo(mat3dpts, CV_8U, 255);
	//img_segmentor.DoSegmentation(mat3dpts);
	//imshow("seg", img_segmentor.m_segImg);
	//rgbdtool.SavePointsToOBJ("demo.obj", mat3dpts);
	//rgbdtool.ComputeNormals(mat3dpts, normals);
	//demo.RunObjSegProposal(cimg, dmap, Mat());
	waitKey(0);
	//demo.RunVideoDemo(SENSOR_KINECT, DEMO_OBJECT_SEG);

	return 0;
	
	/*visualsearch::common::tools::RGBDTools rgbdtool;
	dmap = imread("D:\\imgs\\depth.png", CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	Mat pts;
	rgbdtool.KinectDepthTo3D(dmap, pts);
	rgbdtool.SavePointsToOBJ("d:\\test.obj", pts);
	return 0;*/


	segmentor.PrepareMergerTrainingSamples();
	getchar();
	return 0;

	
	cimg = imread("d:\\imgs\\img_0263.png");
	dmap = imread("D:\\imgs\\depth.png", CV_LOAD_IMAGE_UNCHANGED);
	dmap.convertTo(dmap, CV_32F);
	Mat pts;
	vector<SuperPixel> sps;
	segmentor.verbose = true;
	segmentor.Init(cimg, dmap);
	segmentor.Run();
	segmentor.SaveResults("E:\\res\\seg_prop\\", cimg, sps);
	//rgbdtool.KinectDepthTo3D(dmap, pts);
	//prop.Compute3DDistMap(dmap, Mat());
	//rgbdtool.SavePointsToOBJ("d:\\test.obj", pts);
	//return 0;

	demo.RunVideoDemo(SENSOR_KINECT, DEMO_VIEW_ONLY);
	return 0;
	//demo.RunVideoDemo(SENSOR_KINECT, DEMO_OBJECT_SEG);
	
	waitKey(0);
	//demo.RunObjWinProposal();
	return 0;

	visualsearch::processors::attention::ObjectRanker ranker;
	ranker.LearnObjectWindowPredictor();
	getchar();
	return 0;

	// get input
	visualsearch::io::dataset::NYUDepth2DataMan nyuman;
	FileInfos imgfiles, dmapfiles;
	nyuman.GetImageList(imgfiles);
	nyuman.GetDepthmapList(dmapfiles);
	Mat img = imread(imgfiles[5].filepath);
	nyuman.LoadDepthData(dmapfiles[5].filepath, dmap);
	resize(img, img, Size(300,300));
	resize(dmap, dmap, Size(300,300));
	normalize(dmap, dmap, 1, 0, NORM_MINMAX);

	

	// process
	objectproposal::ObjSegmentProposal processor;
	vector<SuperPixel> res;
	processor.Run(img, dmap, 10, res);
	processor.VisProposals(img, res);
	
	waitKey(0);
	return 0;
}

