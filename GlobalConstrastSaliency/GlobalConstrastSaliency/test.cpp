


#include "Saliency.h"

int main()
{

	Mat img = imread("E:\\Images\\1_25_25257.jpg");
	img.convertTo(img, CV_32FC3, 1.0/255);


	double start_t = getTickCount();

	Saliency detector;
	Mat hc_res = detector.GetHC(img);
	cout<<hc_res.channels()<<endl;

	double diff_t = getTickCount() - start_t;
	cout<<"Time cost: "<<diff_t / getTickFrequency()<<" s"<<endl;

	imshow("Img", img);
	imshow("HC", hc_res);
	waitKey(0);

	return 0;

}