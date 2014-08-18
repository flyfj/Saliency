//////////////////////////////////////////////////////////////////////////
// simplified version

#pragma once

#include <opencv2/opencv.hpp>
#include <map>
#include <functional>
using namespace std;
using namespace cv;

typedef vector<string> vecS;
typedef vector<int> vecI;
typedef vector<float> vecF;
typedef vector<double> vecD;
typedef pair<double, int> CostIdx;
typedef pair<float, int> CostfIdx;

template<typename T> inline T sqr(T x) { return x * x;}
template<class T> inline T vecDist3(const Vec<T, 3> &v1, const Vec<T, 3> &v2) {return sqrt(sqr(v1[0] - v2[0])+sqr(v1[1] - v2[1])+sqr(v1[2] - v2[2]));}
template<class T> inline T vecSqrDist3(const Vec<T, 3> &v1, const Vec<T, 3> &v2) {return sqr(v1[0] - v2[0])+sqr(v1[1] - v2[1])+sqr(v1[2] - v2[2]);}
template<class T1, class T2> inline void operator /= (Vec<T1, 3> &v1, const T2 v2) { v1[0] /= v2; v1[1] /= v2; v1[2] /= v2; }
template<class T> inline T pntSqrDist(const Point_<T> &p1, const Point_<T> &p2) {return sqr(p1.x - p2.x) + sqr(p1.y - p2.y);}



class Saliency
{
private:

	// Histogram based Contrast
	void GetHC(const Mat &binColor3f, const Mat &weights1f, Mat &colorSaliency);

public:
	Saliency(void);
	~Saliency(void);

	// Histogram Contrast of [3]
	Mat GetHC(const Mat &img3f);

	// Color quantization
	int Quantize(const Mat& img3f, Mat &idx1i, Mat &_color3f, Mat &_colorNum, double ratio = 0.95);

	void SmoothSaliency(const Mat &binColor3f, Mat &sal1d, float delta, const vector<vector<CostfIdx>> &similar);

};



