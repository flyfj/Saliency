#include "SaliencyLearner.h"


SaliencyLearner::SaliencyLearner(void)
{
	newImgSize = cv::Size(300,300);
}

//////////////////////////////////////////////////////////////////////////

bool SaliencyLearner::Init(const cv::Mat& colorImg)
{
	return true;
}


bool SaliencyLearner::Preprocess(const cv::Mat& colorImg)
{
	// resize
	cv::Mat img;
	cv::resize(colorImg, img, newImgSize);
	cv::imshow("input", img);
	cv::waitKey(10);

	// do oversegmentation
	spNum = segmentor.DoSegmentation(img);
	cout<<"Superpixel number: "<<spNum<<endl;
	cv::imshow("seg", segmentor.m_segImg);
	cv::waitKey(10);
	sps = segmentor.superPixels;

	// compute feature for each superpixel
	ColorFeatParams cparams(COLOR_LAB, COLOR_FEAT_HIST, true);
	for(size_t i=0; i<sps.size(); i++)
	{
		sps[i].feats.resize(1);
		colorfeat.ComputeColorHistogram(img, sps[i].feats[0], cparams, sps[i].mask);
	}

	// compute self-similarity map for each superpixel
	spSimMat.create(spNum, spNum, CV_32F);
	spSimMat.setTo(0);
	float maxval = 0;
	for(int i=0; i<spSimMat.rows; i++)
	{
		for(int j=i; j<spSimMat.cols; j++)
		{
			spSimMat.at<float>(i, j) = spSimMat.at<float>(j, i) = compareHist(sps[i].feats[0], sps[j].feats[0], CV_COMP_INTERSECT);
		}
	}

	// normalize
	
	//cout<<spSimMat<<endl;

	return true;
}

bool SaliencyLearner::ComputeSelfSimMap(int idx, cv::Mat& feat, cv::Mat& outmap)
{
	outmap.create(sps[idx].mask.rows, sps[idx].mask.cols, CV_32F);
	outmap.setTo(0);

	for(size_t i=0; i<spSimMat.cols; i++)
		outmap.setTo(spSimMat.at<float>(idx, i), sps[i].mask);

	//cv::normalize(outmap, outmap, 1, 0, cv::NORM_MINMAX);

	//cout<<spSimMat.row(idx)<<endl;
	
	// make grids on outmap to downsample
	cv::Size gridDim(20, 20);
	cv::Size gridSize(outmap.cols / gridDim.width, outmap.rows / gridDim.height);
	feat.create(1, gridDim.width*gridDim.height, CV_32F);
	for(int i=0; i<gridDim.height; i++)
	{
		for(int j=0; j<gridDim.width; j++)
		{
			cv::Rect box(j*gridSize.width, i*gridSize.height, gridSize.width, gridSize.height);
			float meanval = cv::mean(outmap(box)).val[0];
			outmap(box).setTo(meanval);

			feat.at<float>(0, i*gridDim.width+j) = meanval;
		}
	}

	return true;
}

bool SaliencyLearner::LoadTrainingData(string imgfoler, string maskfolder)
{
	FileInfos imgfiles;
	tools::ToolFactory::GetFilesFromDir(maskfolder, "*.bmp", imgfiles);

	// 0: negative; 1: positive
	trainSamples.objects.resize(2);

	int len = MIN(1000, imgfiles.size());
	for(size_t i=0; i<len; i++)
	{
		string filename = imgfiles[i].filename.substr(0, imgfiles[i].filename.length()-4);
		cv::Mat img = cv::imread(imgfoler + filename + ".jpg");
		cv::Mat mask = cv::imread(imgfiles[i].filepath, 0);
		if(img.empty() || mask.empty())
			continue;

		cv::resize(mask, mask, newImgSize);
		cv::imshow("foreground", mask);
		cv::waitKey(10);

		// process
		Preprocess(img);

		// compute features
		for(size_t k=0; k<sps.size(); k++)
		{
			cv::Mat feat, outmap;
			ComputeSelfSimMap(k, feat, outmap);

			// check if positive
			float maskArea = cv::countNonZero(mask & sps[k].mask);
			if( maskArea / sps[k].area > 0.9f )
			{
				// positive sample in object
				trainSamples.objects[1].visual_desc.img_desc.push_back(feat);
			}
			if( maskArea / sps[k].area < 0.2f )
			{
				// negative sample
				trainSamples.objects[0].visual_desc.img_desc.push_back(feat);
			}

			cv::imshow("mask", sps[k].mask*255);
			cv::imshow("smap", outmap);
			cv::waitKey(10);

		}

		printf("Loaded image %d.\n", i);

		cv::destroyAllWindows();
	}

	cout<<"Total positive sample: "<<trainSamples.objects[1].visual_desc.img_desc.rows<<endl;
	cout<<"Total negative sample: "<<trainSamples.objects[0].visual_desc.img_desc.rows<<endl;

	return true;
}

bool SaliencyLearner::Train(string savefile, bool ifLoad)
{
	if( !ifLoad )
	{
		learners::DTreeTrainingParams params;
		params.MaxLevel = 10;
		params.min_samp_num = 30;
		params.feature_num = 500;
		params.th_num = 50;

		// create labels
		cv::Mat labels;
		cv::Mat tmpLabels(trainSamples.objects[0].visual_desc.img_desc.rows, 1, CV_32S);
		tmpLabels.setTo(0);
		labels.push_back(tmpLabels);
		tmpLabels.create(trainSamples.objects[1].visual_desc.img_desc.rows, 1, CV_32S);
		tmpLabels.setTo(1);
		labels.push_back(tmpLabels);

		cv::Mat samps;
		samps.push_back( trainSamples.objects[0].visual_desc.img_desc );
		trainSamples.objects[0].visual_desc.img_desc.release();
		samps.push_back( trainSamples.objects[1].visual_desc.img_desc );
		trainSamples.objects[0].visual_desc.img_desc.release();

		// train
		dtree.TrainTree(samps, labels, params);

		// evaluate
		dtree.EvaluateDecisionTree(samps, labels, 2);

		// save to file
		if( !dtree.Save(savefile) )
			return false;
	}
	else
	{
		if( !dtree.Load(savefile) )
			return false;
	}


	return true;
}

bool SaliencyLearner::Run(const cv::Mat& colorImg, cv::Mat& salmap, bool verbose)
{
	// resize
	cv::Mat img;
	cv::resize(colorImg, img, newImgSize);

	salmap.create(newImgSize.height, newImgSize.width, CV_32F);
	salmap.setTo(0);

	// test each superpixel to generate saliency map
	Preprocess(img);

	for(size_t i=0; i<sps.size(); i++)
	{
		cv::Mat feat, outmap;
		ComputeSelfSimMap(i, feat, outmap);

		vector<double> scores;
		dtree.Predict(feat, scores);
		cout<<scores[0]<<" "<<scores[1]<<endl;
		salmap.setTo(scores[1], sps[i].mask);

		if(verbose)
		{
			cv::imshow("cur_mask", sps[i].mask*255);
			cv::imshow("ssmap", outmap);
			cv::waitKey(0);
		}
		
	}


	return true;
}
