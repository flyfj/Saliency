// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning(disable: 4996)
#pragma warning(disable: 4819)

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <time.h>
#include <queue>
#include <omp.h>
#include <Windows.h>
using namespace std;


// OpenCV
#include <cvaux.h>
#include <highgui.h>
#ifdef _DEBUG
#pragma comment(lib, "cxcore210d.lib")
#pragma comment(lib, "cv210d.lib")
#pragma comment(lib, "highgui210d.lib")
#else
#pragma comment(lib, "cxcore210.lib")
#pragma comment(lib, "cv210.lib")
#pragma comment(lib, "highgui210.lib")
#endif // _DEBUG
using namespace cv;

typedef vector<string> vecS;
typedef vector<int> vecI;
typedef vector<float> vecF;
typedef vector<double> vecD;
typedef pair<double, int> CostIdx;
typedef pair<float, int> CostfIdx;

#include "Segmentation/segment-image.h"
#include "HelpFunc/CmLog.h"
#include "HelpFunc/CmTimer.h"
#include "HelpFunc/CmFile.h"


template<typename T> inline T sqr(T x) { return x * x;}
template<class T> inline T vecDist3(const Vec<T, 3> &v1, const Vec<T, 3> &v2) {return sqrt(sqr(v1[0] - v2[0])+sqr(v1[1] - v2[1])+sqr(v1[2] - v2[2]));}
template<class T> inline T vecSqrDist3(const Vec<T, 3> &v1, const Vec<T, 3> &v2) {return sqr(v1[0] - v2[0])+sqr(v1[1] - v2[1])+sqr(v1[2] - v2[2]);}
template<class T1, class T2> inline void operator /= (Vec<T1, 3> &v1, const T2 v2) { v1[0] /= v2; v1[1] /= v2; v1[2] /= v2; }
template<class T> inline T pntSqrDist(const Point_<T> &p1, const Point_<T> &p2) {return sqr(p1.x - p2.x) + sqr(p1.y - p2.y);}


#define CV_Assert_(expr, args) \
{\
	if(!(expr)) {\
	string msg = cv::format args; \
	CmLog::LogError("%s in %s:%d\n", msg.c_str(), __FILE__, __LINE__); \
	cv::error(cv::Exception(CV_StsAssert, msg, __FUNCTION__, __FILE__, __LINE__) ); }\
}
