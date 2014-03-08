

#include "SaliencyLearner.h"

int main()
{
	//string imgfile = "imgs\\0_0_818.jpg";
	string imgfile = "E:\\Datasets\\Saliency\\MSRA\\ImageB\\Image\\6\\6_171_171506.jpg";
	cv::Mat img = cv::imread(imgfile);
	if(img.empty())
	{
		std::cerr<<"No image found."<<std::endl;
		return -1;
	}

	string imgfolder = "E:\\Datasets\\Saliency\\MSRA\\ImageB\\";
	string maskfolder = "E:\\Datasets\\Saliency\\MSRA\\binarymasks\\";
	string modelfile = "tree.model";

	SaliencyLearner salLearner;

	//salLearner.LoadTrainingData(imgfolder, maskfolder);
	salLearner.Train(modelfile, true);

	cv::Mat salmap;
	salLearner.Run(img, salmap, false);
	cv::imshow("map", salmap);

	waitKey(0);

	return 0;
}