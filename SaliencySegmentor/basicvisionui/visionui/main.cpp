

#include "SaliencySegmentor.h"
using namespace Saliency;


SaliencySegmentor sal_segmentor;
Mat img;

void onMouse( int event, int x, int y, int, void* )
{
	if( event != CV_EVENT_LBUTTONDOWN )
		return;

	float score;
	if( !sal_segmentor.MineSalientObjectFromSegment(img, sal_segmentor.GetSegIdByLocation(Point(x,y)), score) )
		return;

}

int main()
{
	img = imread("D:\\imgs\\0_1_1004.jpg");
	imshow("img", img);

	
	sal_segmentor.Init(img);
	imshow("seg", sal_segmentor.GetSegmentImage());
	waitKey(0);

	setMouseCallback("img", onMouse);


	/*Mat sal_map;
	sal_segmentor.ComputeSaliencyMap(img, sal_map);
	normalize(sal_map, sal_map, 1, 0, NORM_L1);
	sal_map = sal_map * 255;
	Mat sal_map_disp;
	sal_map.convertTo(sal_map_disp, CV_8U, 1, 0);
	
	imshow("Saliency map", sal_map_disp);*/
	waitKey(0);

	return 0;
}