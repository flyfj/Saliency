

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
	img = imread("cup.jpg");

	Mat img_rz;
	resize(img, img_rz, Size(400,300));

	imshow("img", img_rz);

	
	sal_segmentor.Init(img_rz);
	imshow("seg", sal_segmentor.GetSegmentImage());
	waitKey(0);

	setMouseCallback("img", onMouse);

	//sal_segmentor.MineSalientObjectsByMergingPairs(img);

	Mat sal_map;
	sal_segmentor.ComputeSaliencyMapByBGPropagation(img_rz, sal_map);
	normalize(sal_map, sal_map, 1, 0, NORM_INF);
	sal_map = sal_map * 255;
	Mat sal_map_disp;
	sal_map.convertTo(sal_map_disp, CV_8U, 1, 0);

	imshow("Saliency map", sal_map_disp);
	waitKey(0);

	return 0;
}