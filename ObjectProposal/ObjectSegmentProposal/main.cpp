// ObjectSegmentProposal.cpp : Defines the entry point for the console application.
//
#include "ObjectProposalTester.h"
#include "ObjSegmentProposal.h"
#include "ObjectRanker.h"
#include "ObjProposalDemo.h"


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
#pragma comment(lib, "EngineLib_debug.lib")

#else
//#pragma comment(lib, "pcl_common_release.lib")
//#pragma comment(lib, "pcl_features_release.lib")
//#pragma comment(lib, "pcl_filters_release.lib")
//#pragma comment(lib, "pcl_visualization_release.lib")
//#pragma comment(lib, "pcl_search_release.lib")
//#pragma comment(lib, "pcl_kdtree_release.lib")
//#pragma comment(lib, "pcl_keypoints_release.lib")
//#pragma comment(lib, "pcl_registration_release.lib")
//#pragma comment(lib, "pcl_io_release.lib")
//#pragma comment(lib, "pcl_ml_release.lib")
//#pragma comment(lib, "pcl_recognition_release.lib")
#pragma comment(lib, "opencv_core249.lib")
#pragma comment(lib, "opencv_imgproc249.lib")
#pragma comment(lib, "opencv_highgui249.lib")
#pragma comment(lib, "opencv_ml249.lib")
#pragma comment(lib, "opencv_features2d249.lib")
#pragma comment(lib, "opencv_nonfree249.lib")
#pragma comment(lib, "opencv_objdetect249.lib")
#pragma comment(lib, "opencv_flann249.lib")
#pragma comment(lib, "EngineLib_release.lib")

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
	ObjectProposalTester tester;
	//tester.TestRankerLearner();
	//tester.ProposalDemo();
	//tester.TestSegment();
	//tester.TestViewMatch();
	//tester.TestGraphcut();
	//tester.TestPatchMatcher();
	//tester.TestSuperpixelClf(false);
	//tester.TestObjSearch();
	//tester.TestSaliency();
	//tester.BoundaryPlayground();
	//tester.TestSegmentor3D();
	//tester.Build3DPCL(DB_NYU2_RGBD);
	//tester.Random();
	//tester.EvaluateOnDataset(DB_SALIENCY_RGBD);
	tester.BatchProposal();
	//tester.TestBoundaryClf(true);
	//cout<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<<endl;
	waitKey(0);
	getchar();
	return 0;
}

