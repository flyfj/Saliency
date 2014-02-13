#ifndef _BIAS
#define _BIAS
#include <opencv2/opencv.hpp>

void calcPbBoundaryWtDisparity(IplImage* , IplImage* , IplImage* , int );

void calcPbBoundaryWtObjMask(IplImage* , IplImage* , IplImage* , IplImage*, int );

void calcPbBoundaryWtFlow(IplImage* , IplImage* , IplImage* , IplImage* , IplImage* ,int );

void calcPbBoundaryWtSurface(IplImage* , IplImage* , IplImage* , IplImage* , double* ,const int , const int , const int , const int );

void getColorHist(IplImage** , const int, double* , const int, const int, const int);
#endif
