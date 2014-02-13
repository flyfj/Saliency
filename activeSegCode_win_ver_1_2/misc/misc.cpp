//----
#include "misc.h" 
#include <opencv2/opencv.hpp>
#include <math.h>
#include <cstdio>
void selectFixPt(int event, int x, int y, int flags, void* fixPt){
  CvPoint* fixPt_local = (CvPoint *)fixPt;
  
  if(event == CV_EVENT_LBUTTONDOWN){
    fixPt_local->x  = x;
    fixPt_local->y  = y;
  }
  
}

void bgrToc1c2c3(IplImage* bgrImg_float, IplImage* c1c2c3Img_float){
	for(int y=0; y < bgrImg_float->height; y++){
		for(int x=0; x <bgrImg_float->width; x++){
			float B = CV_IMAGE_ELEM(bgrImg_float, float, y, 3*x);
			float G = CV_IMAGE_ELEM(bgrImg_float, float, y, 3*x+1);
			float R = CV_IMAGE_ELEM(bgrImg_float, float, y, 3*x+2);
			CV_IMAGE_ELEM(c1c2c3Img_float, float, y, 3*x)   = atan(R/(std::max(G,B)+0.000000000001));
			CV_IMAGE_ELEM(c1c2c3Img_float, float, y, 3*x+1) = atan(G/(std::max(R,B)+0.000000000001));
			CV_IMAGE_ELEM(c1c2c3Img_float, float, y, 3*x+2) = atan(B/(std::max(G,R)+0.000000000001));
		}
	}

} 
