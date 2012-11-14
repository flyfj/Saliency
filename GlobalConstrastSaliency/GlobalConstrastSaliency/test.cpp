


#include "Saliency.h"

int main()
{

	Mat img = imread("D:\\Pictures\\2886800276_4ded411d28.jpg");
	img.convertTo(img, CV_32FC3, 1.0/255);


	double start_t = GetTickCount();

	Saliency detector;
	Mat hc_res = detector.GetHC(img);

	double diff_t = GetTickCount() - start_t;
	cout<<"Time cost: "<<diff_t / getTickFrequency()<<" s"<<endl;

	imshow("Img", img);
	imshow("HC", hc_res);
	waitKey(0);

	return 0;

}