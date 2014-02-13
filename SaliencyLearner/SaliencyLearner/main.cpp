

#include "SaliencyLearner.h"

int main()
{
	string imgfile = "imgs\\0_0_77.jpg";
	cv::Mat img = cv::imread(imgfile);

	string imgfolder = "F:\\Datasets\\Saliency\\MSRA\\ImageB\\";
	string maskfolder = "F:\\Datasets\\Saliency\\MSRA\\binarymasks\\";
	string modelfile = "tree.model";

	SaliencyLearner salLearner;
	salLearner.LoadTrainingData(imgfolder, maskfolder);

	salLearner.Train(modelfile, false);

	cv::Mat salmap;
	salLearner.Run(img, salmap);
	cv::imshow("map", salmap);

	waitKey(0);

	return 0;
}