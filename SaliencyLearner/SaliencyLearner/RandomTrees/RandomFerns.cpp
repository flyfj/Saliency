#include "RandomFerns.h"

namespace learners
{

	bool Fern::Train(const vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const FernTrainingParams& params)
	{
			if(samp_imgs.empty() || labels.empty() || samp_imgs.size() != labels.rows)
			{
					std::cerr<<"Invalid training data."<<std::endl;
					return false;
			}

			// init data
		  double maxlabel, minlabel;
			cv::minMaxLoc(labels, &minlabel, &maxlabel);
			class_likelihood.resize((int)maxlabel+1);
			int max_feat_id = (int)std::pow(2.f, params.num_features);
			for(size_t i=0; i<class_likelihood.size(); i++)
			{
					class_likelihood[i] = NodeStatisticsHist(max_feat_id);
			}

			// generate random features
			features.resize(params.num_features);

			// compute node statistics for each feature evaluation
			for(size_t id=0; id<samp_imgs.size(); id++)
			{
					int feat_val = 0;
					for(size_t feat_id=0; feat_id=features.size(); feat_id++)
					{
							double response = features[feat_id].GetFeatureResponse(samp_imgs[id]);
							if(response > 0)
									feat_val = feat_val<<1 | 1;
					}
					
					// add to hist
					class_likelihood[labels.at<int>(id)].AddSample(feat_val, 1);
			}

			std::cout<<"Finish training fern."<<std::endl;

			m_ifTrained = true;
	}


	int Fern::PredictImage(const cv::Mat& samp_img, vector<double>& all_scores)
	{
			if( !m_ifTrained )	
			{
					std::cerr<<"Train the fern first."<<std::endl;
					return -1;
			}

			// apply to all random feature
			int feat_val = 0;
			for(size_t feat_id=0; feat_id=features.size(); feat_id++)
			{
			 	 double response = features[feat_id].GetFeatureResponse(samp_img);
				 if(response > 0)
					  	feat_val = feat_val<<1 | 1;
		  }

			NodeStatisticsHist class_scores(class_likelihood.size());
			for(size_t i=0; i<class_likelihood.size(); i++)
			{
					// add class i sample number for feat_val
					class_scores.AddSample(i, class_likelihood[i].bins[feat_val]);
			}

			all_scores = class_scores.GetAllProbability();
			return class_scores.GetMaxBinId();
	}


	///////////////////////////////////////////////
		
	bool RandomFerns::Train(const vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const RandomFernsTrainingParams& params)
	{
			if(samp_imgs.empty() || labels.empty() || samp_imgs.size() != labels.rows)
			{
					std::cerr<<"Invalid training data."<<std::endl;
					return false;
			}

			// one training method is just train each fern individually
			// another one is to generate the whole feature pool at once, and assign differnet part to each fern
			// we try first one here
			ferns.resize(params.num_ferns);

			for(size_t fern_id=0; fern_id<ferns.size(); fern_id++)
			{
					if( !ferns[fern_id].Train(samp_imgs, labels, params.fernParams) )
					{
							std::cerr<<"Can't train fern "<<fern_id<<std::endl;
							return false;
					}
			}

			std::cout<<"Finish training random ferns."<<std::endl;

			return true;
	}


	int RandomFerns::PredictImage(const cv::Mat& samp_img, vector<double>& all_scores)
	{
			if( !m_ifTrained )	
			{
					std::cerr<<"Train random ferns first."<<std::endl;
					return -1;
			}

			// get prediction from each fern first
			std::vector<double> cur_fern_score;
			double sum_val = 0;
			std::vector<vector<double>> scores_of_each_fern(ferns.size());
			for(size_t i=0; i<ferns.size(); i++)
			{
					ferns[i].PredictImage(samp_img, cur_fern_score);
					if(i==0)
					{
							all_scores = cur_fern_score;
					}
					else
					{
								// multiple to each channel
								for(size_t j=0; j<cur_fern_score.size(); j++)
										all_scores[j] *= cur_fern_score[j];
					}
			}

			// normalize
			for(size_t i=0; i<all_scores.size(); i++)
					sum_val += all_scores[i];
			
			double max_val = 0;
			int max_id = -1;
			for(size_t i=0; i<all_scores.size(); i++)
			{
					all_scores[i] /= sum_val;
					if(all_scores[i] > max_val)
					{
							max_val = all_scores[i];
							max_id = i;
					}
			}

			return max_id;
	}


}


