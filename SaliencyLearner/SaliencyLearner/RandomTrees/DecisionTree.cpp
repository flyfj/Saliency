#include "DecisionTree.h"


namespace learners
{
	//////////////////////////////////////////////////////////////////////////
	double NodeStatisticsHist::Entropy() const
	{
		if (sampleCount == 0)
			return 0.0;

		// modified
		// sum of weighted sample number
		// compute normalized weights of each sample
		double result = 0.0;
		for (int b = 0; b < bins.size(); b++)
		{
			double p = class_weights[b] / weight_sum;
					//(double)bins[b] / (double)sampleCount;
			result -= p == 0.0 ? 0.0 : p * log(p)/log(2.0);
		}

		return result;
	}

	NodeStatisticsHist::NodeStatisticsHist()
	{
		bins.clear();
		class_weights.clear();
		sampleCount = 0;
		weight_sum = 0;
	}

	NodeStatisticsHist::NodeStatisticsHist(int nClasses)
	{
		bins.clear();
		bins.resize(nClasses, 0);
		class_weights.clear();
		class_weights.resize(nClasses, 0);

		weight_sum = 0;
		sampleCount = 0;
	}

	double NodeStatisticsHist::GetProbability(int classIndex) const
	{
		if(classIndex >= bins.size())
			return -1;

		// do smoothed normalization; every class probability should be non-zero
		// p(C) = (N_c+N_r)/(N+#C*N_r), N_r=1
		return (bins[classIndex]+1) / (sampleCount+bins.size());

		/*if(sampleCount <= 0)
				return 0;
		else
				return bins[classIndex] / sampleCount;*/
	}

	double NodeStatisticsHist::GetWeightedProbability(int classIndex) const
	{
		if(classIndex >= bins.size())
			return -1;

		return (class_weights[classIndex]+1) / (weight_sum+class_weights.size());
		/*if(weight_sum <= 0)
				return 0;
		else
				return class_weights[classIndex] / weight_sum;*/
	}

	int NodeStatisticsHist::GetMaxBinId() const
	{
		double max_num = bins[0];
		int max_id = 0;
		for(size_t i=1; i<bins.size(); i++)
			if(bins[i] > max_num)
			{
				max_num = bins[i];
				max_id = i;
			}

		return max_id;
	}

	int NodeStatisticsHist::GetMaxWeightedBinId() const
	{
			double max_num = class_weights[0];
		int max_id = 0;
		for(size_t i=1; i<class_weights.size(); i++)
			if(class_weights[i] > max_num)
			{
				max_num = class_weights[i];
				max_id = i;
			}

		return max_id;
	}

	double NodeStatisticsHist::GetMaxProbability() const
	{
		return GetProbability( GetMaxBinId() );
	}

	double NodeStatisticsHist::GetMaxWeightedProbability() const
	{
			return (double)GetWeightedProbability( GetMaxWeightedBinId() );
	}

	std::vector<double> NodeStatisticsHist::GetAllProbability() const
	{
		std::vector<double> scores = bins;
		for(size_t i=0; i<bins.size(); i++)
			scores[i] = GetProbability(i);

		return scores;
	}

	std::vector<double> NodeStatisticsHist::GetAllWeightedProbability() const
	{
		std::vector<double> scores = class_weights;
		for(size_t i=0; i<class_weights.size(); i++)
			scores[i] = GetWeightedProbability(i);

		return scores;
	}

	double NodeStatisticsHist::ComputeInformationGain(const NodeStatisticsHist& parent, 
		const NodeStatisticsHist& leftChild, const NodeStatisticsHist& rightChild)
	{
		double entropyBefore = parent.Entropy();

		unsigned int nTotalSamples = leftChild.SampleNum() + rightChild.SampleNum();

		if (nTotalSamples <= 1)
			return 0.0;

		double entropyAfter = (leftChild.SampleNum() * leftChild.Entropy() + 
			rightChild.SampleNum() * rightChild.Entropy()) / nTotalSamples;

		return entropyBefore - entropyAfter;
	}

	double NodeStatisticsHist::ComputeNegativeRejection(const NodeStatisticsHist& parent, 
			const NodeStatisticsHist& leftChild, const NodeStatisticsHist& rightChild)
	{
			// measure negative sample rejection rate
			// 1. #N / #N_P: more negatives from parent node go to one of the child nodes
			// 2. #N / #A: higher percentage in current child node
			// 3. compute 1 * 2 for both nodes and select higher one as measurement
			// measure positive ratio at the same time to enforce even separation
			double neg_1 = leftChild.GetProbability(0) * leftChild.SampleNum();
			double neg_2 = rightChild.GetProbability(0) * rightChild.SampleNum();
			double pos_1 = leftChild.GetProbability(1) * leftChild.SampleNum();
			double pos_2 = rightChild.GetProbability(1) * rightChild.SampleNum();
			double neg_parent = parent.GetProbability(0) * parent.SampleNum();
			double pos_parent = parent.GetProbability(1) * parent.SampleNum();
			double neg_var_1 = (neg_parent<=0? 0: (neg_1 / neg_parent)) * leftChild.GetProbability(0);
			double neg_var_2 = (neg_parent<=0? 0: (neg_2 / neg_parent)) * rightChild.GetProbability(0);
			double pos_var_1 = (pos_parent<=0? 0: (pos_1 / pos_parent)) * leftChild.GetProbability(1);
			double pos_var_2 = (pos_parent<=0? 0: (pos_2 / pos_parent)) * rightChild.GetProbability(1);

			return MAX(neg_var_1 * pos_var_2, pos_var_1 * neg_var_2);
	}

	// more general interface to evaluate separability
	double NodeStatisticsHist::EvaluateSeparability(const NodeStatisticsHist& parent, 
			const NodeStatisticsHist& leftChild, const NodeStatisticsHist& rightChild, OptimalCriterion opt_crit)
	{
			if(opt_crit == SEP_CRIT_IG)
				return ComputeInformationGain(parent, leftChild, rightChild);
			if(opt_crit == SEP_CRIT_NEG_REJ)
				return ComputeNegativeRejection(parent, leftChild, rightChild);

			return 0;
	}

	void NodeStatisticsHist::AddSamples(const cv::Mat& labels, const std::vector<int>& label_ids, std::vector<double>& weights)
	{
			if(weights.empty())
					weights.resize(label_ids.size(), 1);

		//sampleCount += label_ids.size();
		for(size_t i=0; i<label_ids.size(); i++)
		{
				AddSample(labels.at<int>(label_ids[i]), weights[i]);
			//bins[labels.at<int>(label_ids[i])]++;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	std::vector<PixelPairComparisonFeature> PixelPairComparisonFeature::GenAllFeatures(int num_bins)
	{
			// quantize x and y into num_bins segments
			vector<PixelPairComparisonFeature> feats;
		  vector<cv::Point2f> all_point_locs;
			all_point_locs.reserve(num_bins*num_bins);
		  for(float feat_y = 0; feat_y < 1; feat_y += 1.f/num_bins)
		  {
				  for(float feat_x = 0; feat_x < 1; feat_x += 1.f/num_bins)
				  {
					  	cv::Point2f cur_p(feat_x, feat_y);
						  all_point_locs.push_back(cur_p);
				  }
		  }

			for(size_t i=0; i<all_point_locs.size(); i++)
			{
					for(size_t j=i+1; j<all_point_locs.size(); j++)
					{
							PixelPairComparisonFeature cur_feat(all_point_locs[i], all_point_locs[j]);
							feats.push_back(cur_feat);
					}
			}

			return feats;
	}

	std::vector<PixelPairComparisonFeature> PixelPairComparisonFeature::GenAllCenterFeatures(int num_bins)
	{
			// quantize x and y into num_bins segments
			vector<PixelPairComparisonFeature> feats;
		  vector<cv::Point2f> all_point_locs;
			cv::Point2f center_loc(0.5f, 0.5f);
			all_point_locs.reserve(num_bins*num_bins);
		  for(float feat_y = 0; feat_y < 1; feat_y += 1.f/num_bins)
		  {
				  for(float feat_x = 0; feat_x < 1; feat_x += 1.f/num_bins)
				  {
					  	cv::Point2f cur_p(feat_x, feat_y);
						  all_point_locs.push_back(cur_p);
				  }
		  }

			for(size_t i=0; i<all_point_locs.size(); i++)
			{
						PixelPairComparisonFeature cur_feat(center_loc, all_point_locs[i]);
						feats.push_back(cur_feat);
			}

			return feats;
	}

	//////////////////////////////////////////////////////////////////////////
	DecisionTreeNode::DecisionTreeNode()
	{
		isLeaf = false;
		isSplit = false;
		th = 0;
	}
	
	DecisionTreeNode::DecisionTreeNode(int nClass)
	{
		isLeaf = false;
		isSplit = false;
		th = 0;

		trainingDataStatistics = NodeStatisticsHist(nClass);
	}

	//////////////////////////////////////////////////////////////////////////

	double DecisionTree::easy_sample_ratio = 0;

	bool DecisionTree::ifEasy = false;

	DecisionTree::DecisionTree(void)
	{
		useFastFeature = false;
		m_ifTrained = false;
		tree_feat_type = DTREE_FEAT_PAIR;
	}

	//////////////////////////////////////////////////////////////////////////
	bool DecisionTree::TrainTree(const cv::Mat& samps, const cv::Mat& labels, 
			const DTreeTrainingParams& train_params, std::vector<double>& samp_weights)
	{
			cout<<labels.depth()<<endl;
		if(samps.rows != labels.rows || labels.depth() != CV_32S )
		{
			cerr<<"Invalid decision tree training data format."<<endl;
			return false;
		}

		if(samp_weights.empty() || samp_weights.size() != samps.rows)
		{
				samp_weights.clear();
				samp_weights.resize(samps.rows, 1);
		}

		// init random
		srand(time(NULL));

		// set up some signs
		bool ifNormalizeWeights = false;
		bool ifReweight4EachNode = false;

		// find max label id
		int max_label = 0;
		for(int i=0; i<labels.rows; i++)
			if(labels.at<int>(i) > max_label)
				max_label = labels.at<int>(i);

		// create root node
		DecisionTreeNode rootNode(max_label+1);
		rootNode.nodeId = 0;
		rootNode.nodeLevel = 0;
		rootNode.samp_ids.clear();
		for(int i=0; i<labels.rows; i++)
			rootNode.samp_ids.push_back(i);
		rootNode.trainingDataStatistics.AddSamples(labels, rootNode.samp_ids, samp_weights);
		nodes[0] = rootNode;

		// start to train
		cout<<"Start to train decision tree..."<<endl;
		std::queue<int> toTrainNodes;
		toTrainNodes.push(0);


		while( !toTrainNodes.empty() )
		{
			int cur_node_id = toTrainNodes.front();
			toTrainNodes.pop();

			DecisionTreeNode& cur_node = nodes[cur_node_id];
			// check if leaf
			if(cur_node.isLeaf)
				continue;

			// compute normalized weights for samples in current node: MAX_NORM
			vector<double> norm_weights(cur_node.samp_ids.size());
			double max_weight = 0;

			for(size_t i=0; i<cur_node.samp_ids.size(); i++)
			{
				norm_weights[i] = samp_weights[cur_node.samp_ids[i]];
				if(norm_weights[i] > max_weight)
						max_weight = norm_weights[i];
			}

			if(ifNormalizeWeights && max_weight > 0)
			{
				for(size_t i=0; i<norm_weights.size(); i++)
						norm_weights[i] /= max_weight;
			}

			// randomly choose feature and threshold
			//PixelPairComparisonFeature pair_feature;
			AxisFeature best_feature;
			double best_th = 0;
			double maxGain = 0;//
			for(int feat_id=0; feat_id < train_params.feature_num; feat_id++)
			{
				AxisFeature cur_feat;
				//PixelPairComparisonFeature cur_feat;
				
				if(useFastFeature)
				{
					if(cur_node.nodeLevel < train_params.fastLevel)
					{
						// use only fast feature
						int rand_id = rand() % train_params.fast_feats_dims.size();
						cur_feat.SetAxisId(train_params.fast_feats_dims[rand_id]);
					}
					else
					{
						// use all features
						int rand_id = rand() % samps.cols;
						cur_feat.SetAxisId(rand_id);
					}
				}
				else
				{
					// use all feature channels
					int rand_id = rand() % samps.cols;
					cur_feat.SetAxisId(rand_id);
				}

				
				// apply to all samples
				vector<double> responses(cur_node.samp_ids.size());
				for(size_t i=0; i<cur_node.samp_ids.size(); i++)
				{
					responses[i] = cur_feat.GetFeatureResponse(samps.row(cur_node.samp_ids[i]));
				}


				// choose threshold
				vector<double> random_ths;
				if(responses.size() < train_params.th_num) random_ths = responses;
				else
				{
					for(int i=0; i<train_params.th_num; i++)
					{
						int th_id = rand() % responses.size();
						double cur_th = responses[th_id];
						random_ths.push_back(cur_th);
					}
				}

				// test each th
				for(int i=0; i<random_ths.size(); i++)
				{
					double cur_th = random_ths[i];

					// do separation
					NodeStatisticsHist leftHist(max_label+1);
					NodeStatisticsHist rightHist(max_label+1);
					for(size_t j=0; j<cur_node.samp_ids.size(); j++)
					{
						int cur_samp_id = cur_node.samp_ids[j];
						int cur_label = labels.at<int>(cur_samp_id);
						if(responses[j] < cur_th)
								leftHist.AddSample(cur_label, norm_weights[j]);
								//leftHist.AddSample(cur_label, samp_weights[cur_samp_id]);
						else
								rightHist.AddSample(cur_label, norm_weights[j]);
							//rightHist.AddSample(cur_label, samp_weights[cur_samp_id]);
					}

					// compute information gain
					double ig = NodeStatisticsHist::EvaluateSeparability(cur_node.trainingDataStatistics,
							leftHist, rightHist, train_params.opt_crit);
					if(ig > maxGain)
					{
						maxGain = ig;
						best_feature = cur_feat;
						best_th = cur_th;
					}
				}
			}

			// update current node
			cur_node.axis_feature = best_feature;
			cout<<"best axis: "<<cur_node.axis_feature.GetAxisId()<<endl;
			cur_node.th = best_th;
			cout<<"best th: "<<best_th<<endl;

			// generate children
			DecisionTreeNode leftNode(max_label+1);
			leftNode.nodeId = cur_node_id*2 + 1;
			leftNode.nodeLevel = cur_node.nodeLevel + 1;
			DecisionTreeNode rightNode(max_label+1);
			rightNode.nodeId = cur_node_id*2 + 2;
			rightNode.nodeLevel = cur_node.nodeLevel + 1;
			for(size_t i=0; i<cur_node.samp_ids.size(); i++)
			{
				int cur_samp_id = cur_node.samp_ids[i];
				double response = cur_node.axis_feature.GetFeatureResponse(samps.row(cur_samp_id));
				if(response < cur_node.th)
				{
					leftNode.samp_ids.push_back(cur_samp_id);
					leftNode.trainingDataStatistics.AddSample(labels.at<int>(cur_samp_id), norm_weights[i]); //samp_weights[cur_samp_id]
				}
				else
				{
					rightNode.samp_ids.push_back(cur_samp_id);
					rightNode.trainingDataStatistics.AddSample(labels.at<int>(cur_samp_id), norm_weights[i]);		//samp_weights[cur_samp_id]
				}
			}

			// check if leaf
			if(leftNode.trainingDataStatistics.GetMaxWeightedProbability() >= train_params.STOP_CONF || 
				leftNode.nodeLevel == train_params.MaxLevel || 
				leftNode.trainingDataStatistics.SampleNum() <= train_params.min_samp_num || maxGain <= 0.0000001)
				leftNode.isLeaf = true;
			if(rightNode.trainingDataStatistics.GetMaxWeightedProbability() >= train_params.STOP_CONF || 
				rightNode.nodeLevel == train_params.MaxLevel ||
				rightNode.trainingDataStatistics.SampleNum() <= train_params.min_samp_num || maxGain <= 0.0000001)
				rightNode.isLeaf = true;

			// add nodes to tree
			cout<<leftNode.nodeId<<" "<<rightNode.nodeId<<endl;
			nodes[leftNode.nodeId] = leftNode;
			nodes[rightNode.nodeId] = rightNode;
			toTrainNodes.push(leftNode.nodeId);
			toTrainNodes.push(rightNode.nodeId);
		}

		m_ifTrained = true;
		cout<<"Finish training."<<endl;

		return true;
	}
		
	int DecisionTree::Predict(const cv::Mat& samp, vector<double>& all_scores)
	{
			if(!m_ifTrained)
			{
					std::cerr<<"Tree hasn't been trained."<<std::endl;
					return -1;
			}

		// traverse tree
		int cur_node_id = 0;
		while(1)
		{
			DecisionTreeNode& cur_node = nodes[cur_node_id];

			if(cur_node.isLeaf)
			{
					all_scores = cur_node.trainingDataStatistics.GetAllProbability();
					return cur_node.trainingDataStatistics.GetMaxBinId();
					/*all_scores = cur_node.trainingDataStatistics.GetAllWeightedProbability();
					return cur_node.trainingDataStatistics.GetMaxWeightedBinId();*/
			}

			/*if(cur_node.nodeLevel <= defaultParams.fastLevel)
				cout<<defaultParams.fast_feats_names[cur_node.feature.axisIdx]<<endl;
			if(cur_node.nodeLevel == defaultParams.fastLevel)
			{
				if(cur_node.trainingDataStatistics.GetMaxProbability() > defaultParams.conf_th)
				{
					cout<<"Easy to classify"<<endl;
					ifEasy = true;
					easy_sample_ratio++;
				}
				else
				{
					ifEasy = false;
					cout<<"Hard to classify"<<endl;
				}
			}*/

			double response = cur_node.axis_feature.GetFeatureResponse(samp);
			if(response < cur_node.th)
				cur_node_id = cur_node.nodeId*2 + 1;
			else
				cur_node_id = cur_node.nodeId*2 + 2;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	bool DecisionTree::TrainTree(const vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const DTreeTrainingParams& train_params)
	{
			if(samp_imgs.empty() || labels.empty() )
			{
					cerr<<"Invalid decision tree training data format."<<endl;
					return false;
			}

			// find max label id
			int max_label = 0;
			for(int i=0; i<labels.rows; i++)
					if(labels.at<int>(i) > max_label)
							max_label = labels.at<int>(i);

			// create root node
			max_tree_level = 0;
			DecisionTreeNode rootNode(max_label+1);
			rootNode.nodeId = 0;
			rootNode.nodeLevel = 0;
			rootNode.samp_ids.clear();
			for(int i=0; i<labels.rows; i++)
					rootNode.samp_ids.push_back(i);
			// add all samples
			rootNode.trainingDataStatistics.AddSamples(labels, rootNode.samp_ids);
			nodes[0] = rootNode;

			// start to train
			cout<<"Start to train decision tree..."<<endl;
			queue<int> toTrainNodes;
			toTrainNodes.push(0);
			while( !toTrainNodes.empty() )
			{
					int cur_node_id = toTrainNodes.front();
					toTrainNodes.pop();

					DecisionTreeNode& cur_node = nodes[cur_node_id];
					if(cur_node.isLeaf)
							continue;

					// try every feature and random threshold
					PixelPairComparisonFeature best_feature;
					double best_th = 0;
					double maxGain = 0;

					// generate all features
					vector<PixelPairComparisonFeature> all_feats = PixelPairComparisonFeature::GenAllFeatures(20);
					//PixelPairComparisonFeature::GenAllCenterFeatures(20);

					// test each feature
					for(int feat_id = 0; feat_id < all_feats.size(); feat_id++)
					{
								PixelPairComparisonFeature& cur_feat = all_feats[feat_id];

								// apply to all samples
								vector<double> responses(cur_node.samp_ids.size());
								for(size_t i=0; i<cur_node.samp_ids.size(); i++)
								{
										int cur_samp_id = cur_node.samp_ids[i];
										responses[i] = cur_feat.GetFeatureResponse(samp_imgs[cur_samp_id]);
								}

								// choose threshold
								vector<double> random_ths;
								//for(int i=0; i<=128; i+=10)
										random_ths.push_back(30);

								// test each th
								for(int i=0; i<random_ths.size(); i++)
								{
										double cur_th = random_ths[i];

										// do separation
										NodeStatisticsHist leftHist(max_label+1);
										NodeStatisticsHist rightHist(max_label+1);
										for(size_t j=0; j<cur_node.samp_ids.size(); j++)
										{
												int cur_label = labels.at<int>(cur_node.samp_ids[j]);
												if(responses[j] < cur_th)
														leftHist.AddSample(cur_label, 1);
												else
														rightHist.AddSample(cur_label, 1);
										}

										// compute information gain
										double ig = NodeStatisticsHist::EvaluateSeparability(cur_node.trainingDataStatistics,
												leftHist, rightHist, SEP_CRIT_IG);
										if(ig > maxGain)
										{
												maxGain = ig;
												best_feature = cur_feat;
												best_th = cur_th;
										}
								}
					}

					// update current node
					cur_node.pair_feature = best_feature;
					cout<<"Feature: "<<cur_node.pair_feature<<endl;
					cout<<"Th: "<<best_th<<endl;
					cur_node.th = best_th;

					// generate children
					DecisionTreeNode leftNode(max_label+1);
					leftNode.nodeId = cur_node_id*2 + 1;
					leftNode.nodeLevel = cur_node.nodeLevel + 1;
					if(leftNode.nodeLevel > max_tree_level)
							max_tree_level = leftNode.nodeLevel;

					DecisionTreeNode rightNode(max_label+1);
					rightNode.nodeId = cur_node_id*2 + 2;
					rightNode.nodeLevel = cur_node.nodeLevel + 1;
					if(rightNode.nodeLevel > max_tree_level)
							max_tree_level = rightNode.nodeLevel;

					// assign every sample
					for(size_t i=0; i<cur_node.samp_ids.size(); i++)
					{
							int cur_samp_id = cur_node.samp_ids[i];
							double response = cur_node.pair_feature.GetFeatureResponse(samp_imgs[cur_samp_id]);
							if(response < cur_node.th)
							{
									leftNode.samp_ids.push_back(cur_samp_id);
									leftNode.trainingDataStatistics.AddSample(labels.at<int>(cur_samp_id), 1);
							}
							else
							{
									rightNode.samp_ids.push_back(cur_samp_id);
									rightNode.trainingDataStatistics.AddSample(labels.at<int>(cur_samp_id), 1);
							}
					}

					// compute measure
					cur_node.quality_measure = NodeStatisticsHist::EvaluateSeparability(cur_node.trainingDataStatistics,
							leftNode.trainingDataStatistics, rightNode.trainingDataStatistics, SEP_CRIT_IG);
					cout<<"Separability: "<<cur_node.quality_measure<<endl;
					if(cur_node.quality_measure < train_params.STOP_DIFF)
					{
							// no need for separation
							cur_node.isLeaf = true;
							continue;
					}

					// check if leaf
					if(leftNode.trainingDataStatistics.GetMaxProbability() > train_params.STOP_CONF || 
							leftNode.nodeLevel == train_params.MaxLevel || 
							leftNode.trainingDataStatistics.SampleNum() <= train_params.min_samp_num)
							leftNode.isLeaf = true;
					if(rightNode.trainingDataStatistics.GetMaxProbability() > train_params.STOP_CONF || 
							rightNode.nodeLevel == train_params.MaxLevel ||
							rightNode.trainingDataStatistics.SampleNum() <= train_params.min_samp_num)
							rightNode.isLeaf = true;

					// add nodes to tree
					cout<<"Left: "<<leftNode.nodeId<<" ";
					for(size_t id=0; id<leftNode.trainingDataStatistics.ClassNum(); id++)
							cout<<(id==0? "":" ")<<leftNode.trainingDataStatistics.bins[id];
					cout<<endl;
					cout<<"Right: "<<rightNode.nodeId<<" ";
					for(size_t id=0; id<rightNode.trainingDataStatistics.ClassNum(); id++)
							cout<<(id==0? "":" ")<<rightNode.trainingDataStatistics.bins[id];
					cout<<endl<<endl;

					nodes[leftNode.nodeId] = leftNode;
					nodes[rightNode.nodeId] = rightNode;
					toTrainNodes.push(leftNode.nodeId);
					toTrainNodes.push(rightNode.nodeId);
			}

			m_ifTrained = true;
			cout<<"Max level: "<<max_tree_level<<endl;
			cout<<"Finish training."<<endl<<endl;

			// measure training performance
			double correct_samp = 0;
			vector<double> all_scores;
			for(size_t i=0; i<samp_imgs.size(); i++)
			{
					int pred_label = PredictImage(samp_imgs[i], all_scores);
					if(pred_label == labels.at<int>(i))
							correct_samp++;
			}
			std::cout<<"Training accuracy: "<<correct_samp / samp_imgs.size()<<std::endl;

			return true;
	}

//#define Verbose

	int DecisionTree::PredictImage(const cv::Mat& img, std::vector<double>& all_scores)
	{
			if(!m_ifTrained)
			{
					std::cerr<<"Tree hasn't been trained."<<std::endl;
					return -1;
			}

#ifdef Verbose

			cv::Size newsize(200, 200);
			cv::Mat disp_img;
			cv::resize(img, disp_img, newsize);
			cv::cvtColor(disp_img, disp_img, CV_GRAY2BGR);

#endif

			// traverse tree
			int cur_node_id = 0;
			while(1)
			{
				DecisionTreeNode& cur_node = nodes[cur_node_id];

				if(cur_node.isLeaf)
				{
					all_scores = cur_node.trainingDataStatistics.GetAllProbability();
					return cur_node.trainingDataStatistics.GetMaxBinId();
				}

				double response = cur_node.pair_feature.GetFeatureResponse(img);
				
#ifdef Verbose

				// visualize test features
				PixelPair cur_pair = cur_node.pair_feature.GetFeature();
				cv::Point pt1, pt2;
				pt1.x = (int)(newsize.width * cur_pair.pair_loc1.x);
				pt1.y = (int)(newsize.height * cur_pair.pair_loc1.y);
				pt2.x = (int)(newsize.width * cur_pair.pair_loc2.x);
				pt2.y = (int)(newsize.height * cur_pair.pair_loc2.y);
				cout<<"P1: "<<pt1.x<<" "<<pt1.y<<" P2: "<<pt2.x<<" "<<pt2.y<<endl;
				cv::circle(disp_img, pt1, 2, CV_RGB(255,0,0));
				cv::circle(disp_img, pt2, 2, CV_RGB(255,0,0));
				cv::line(disp_img, pt1, pt2, CV_RGB(255,0,0), 1.5);
				cv::imshow("feat", disp_img);
				cv::waitKey(0);

#endif

				if(response < cur_node.th)
				{
						//std::cout<<"go left"<<std::endl;
					  cur_node_id = cur_node.nodeId*2 + 1;
				}
				else
				{
						//std::cout<<"go right"<<std::endl;
					  cur_node_id = cur_node.nodeId*2 + 2;
				}
			}
	}

	//////////////////////////////////////////////////////////////////////////

	bool DecisionTree::EvaluateDecisionTree(const cv::Mat& test_samps, const cv::Mat& test_labels, int nClass)
	{
			if(!m_ifTrained || test_samps.rows != test_labels.rows)
					return false;

			// compute confusion matrix
			cv::Mat confusionMat(nClass, nClass, CV_64F);
			confusionMat.setTo(0);
			vector<double> all_scores;
			for(int r=0; r<test_samps.rows; r++)
			{
					int pred_label = Predict(test_samps.row(r), all_scores);
					confusionMat.at<double>(test_labels.at<int>(r), pred_label)++;
			}
			

			// normalize
			double avg_acc = 0;
			for(int r=0; r<confusionMat.rows; r++)
			{
				avg_acc += confusionMat.at<double>(r,r);
				cv::normalize(confusionMat.row(r), confusionMat.row(r), 1, 0, cv::NORM_L1);

				cout<<"Accuracy for class "<<r<<" : "<<confusionMat.at<double>(r,r)<<endl;
			}

			// compute average accuracy (#corr/#samples)
			avg_acc /= test_samps.rows;
			cout<<"Average accuracy: "<<avg_acc<<endl;

			cout<<confusionMat<<endl;

			return true;
	}

	void DecisionTree::DrawPixelDecisionBoundary(int width, int height, int id)
	{
			if( !m_ifTrained )
					return;

			cv::Mat disp_img(height, width, CV_8UC3);
			disp_img.setTo(cv::Vec3b(255,255,255));

			std::vector<double> all_scores;
			cv::Mat samp(1,2,CV_32F);
			for(int r=0; r<height; r++)
			{
					for(int c=0; c<width; c++)
					{
							samp.at<float>(0,0) = c;
							samp.at<float>(0,1) = r;
							int pred_label = Predict(samp, all_scores);
							if(pred_label == 0)
									disp_img.at<cv::Vec3b>(r,c) = cv::Vec3b(255*all_scores[0], 0, 0);//cv::Vec3b(255, 0, 0);
							if(pred_label == 1)
									disp_img.at<cv::Vec3b>(r,c) = cv::Vec3b(0, 0, 255*all_scores[1]);//cv::Vec3b(0, 0, 255);
					}
			}

			char str[10];
			sprintf_s(str, "boundary%d", id);
			cv::imshow(str, disp_img);
			cv::waitKey(10);
	}

	void DecisionTree::DrawPixelWeightMap(const cv::Mat& samps, const std::vector<double>& weights, int id)
	{
			if( !m_ifTrained )
					return;

			cv::Mat disp_img(500, 500, CV_8UC3);
			disp_img.setTo(cv::Vec3b(0,0,0));

			std::vector<double> norm_weights = weights;
			double max_w = 0;
			for(size_t i=0; i<weights.size(); i++)
					if(max_w < weights[i])
							max_w = weights[i];

			if(max_w > 0)
			{
					for(size_t i=0; i<norm_weights.size(); i++)
							norm_weights[i] /= max_w;
			}

			for(int r=0; r<samps.rows; r++)
			{
					//if(norm_weights[r] < 0.5f)	// use green
						disp_img.at<cv::Vec3b>(samps.at<float>(r,1), samps.at<float>(r,0)) = cv::Vec3b(0, 255*(1-norm_weights[r]), 255*norm_weights[r]);
					//else
						//disp_img.at<cv::Vec3b>(samps.at<float>(r,1), samps.at<float>(r,0)) = cv::Vec3b(0, 0, 255*norm_weights[r]);
			}

			char str[10];
			sprintf_s(str, "weight%d", id);
			cv::imshow(str, disp_img);
			cv::waitKey(10);
	}

	//////////////////////////////////////////////////////////////////////////

	bool DecisionTree::Save(const string savefile)
	{
		ofstream out(savefile.c_str());
		if( !out.good() )
		{
			cerr<<"Can't open save file for dtree."<<endl;
			return false;
		}

		out<<max_tree_level<<endl;
		for(map<int, DecisionTreeNode>::const_iterator pi=nodes.begin(); pi!=nodes.end(); pi++)
		{
				// node id
				out<<pi->first<<" "<<pi->second.nodeLevel<<" "<<pi->second.isLeaf<<" ";
				if(tree_feat_type == DTREE_FEAT_AXIS)
						out<<pi->second.axis_feature.GetAxisId();
				else
						out<<pi->second.pair_feature;
				out<<" "<<pi->second.th<<" "
						<<pi->second.trainingDataStatistics.sampleCount<<" "<<pi->second.trainingDataStatistics.bins.size()<<" ";
				for(size_t i=0; i<pi->second.trainingDataStatistics.bins.size(); i++)
				out<<(i==0? "":" ")<<pi->second.trainingDataStatistics.bins[i];
				out<<endl;
		}

		std::cout<<"Tree saved to "<<savefile<<std::endl;

		return true;
	}

	bool DecisionTree::Load(const string loadfile)
	{
		ifstream in(loadfile.c_str());
		if(!in.good())
		{
			cerr<<"Can't open load file for dtree."<<endl;
			return false;
		}

		in>>max_tree_level;
		// read nodes
		int nodeId;
		nodes.clear();
		while(in>>nodeId)
		{
				DecisionTreeNode cur_node;
				cur_node.nodeId = nodeId;
				int axis_id;
				cv::Point binIds;
				in>>cur_node.nodeLevel>>cur_node.isLeaf;
				if(tree_feat_type == DTREE_FEAT_AXIS)
				{
						in>>axis_id;
						cur_node.axis_feature.SetAxisId(axis_id);
				}
				else
						in>>cur_node.pair_feature;
				in>>cur_node.th;
				
				int binSize;
				in>>cur_node.trainingDataStatistics.sampleCount>>binSize;
				cur_node.trainingDataStatistics.bins.resize(binSize);
				for(size_t i=0; i<binSize; i++)
				in>>cur_node.trainingDataStatistics.bins[i];
			
				nodes[nodeId] = cur_node;
		}

		std::cout<<"Tree loaded from "<<loadfile<<std::endl;

		m_ifTrained = true;

		return true;
	}

}

