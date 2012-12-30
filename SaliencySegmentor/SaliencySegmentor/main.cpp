

#include "SaliencySegmentor.h"
using namespace Saliency;


SaliencySegmentor sal_segmentor;
Mat img;

void onMouse( int event, int x, int y, int, void* )
{
	if( event != CV_EVENT_LBUTTONDOWN )
		return;

	if( !sal_segmentor.MineSalientObjectFromSegment(img, sal_segmentor.GetSegIdByLocation(Point(x,y))) )
		return;

}

int main()
{
	img = imread("D:\\imgs\\mario.jpg");
	imshow("img", img);

	
	sal_segmentor.Init(img);
	imshow("seg", sal_segmentor.GetSegmentImage());
	waitKey(0);

	setMouseCallback("img", onMouse);


	waitKey(0);

	return 0;
}