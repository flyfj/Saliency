#include "DecisionTree.h"


namespace learners
{
	//////////////////////////////////////////////////////////////////////////
	double NodeStatisticsHist::Entropy() const
	{
		if (sampleCount == 0)
			return 0.0;

		double result = 0.0;
		for (int b = 0; b < bins.size(); b++)
		{
			double p = (double)bins[b] / (double)sampleCount;
			result -= p == 0.0 ? 0.0 : p * log(p)/log(2.0);
		}

		return result;
	}

	NodeStatisticsHist::NodeStatisticsHist()
	{
		bins.clear();
		sampleCount = 0;
	}

	NodeStatisticsHist::NodeStatisticsHist(int nClasses)
	{
		bins.clear();
		bins.resize(nClasses, 0);
		sampleCount = 0;
	}

	float NodeStatisticsHist::GetProbability(int classIndex) const
	{
		if(classIndex >= bins.size())
			return -1;

		return bins[classIndex] / sampleCount;
	}

	int NodeStatisticsHist::GetMaxBinId() const
	{
		float max_num = bins[0];
		int max_id = 0;
		for(size_t i=1; i<bins.size(); i++)
			if(bins[i] > max_num)
			{
				max_num = bins[i];
				max_id = i;
			}

		return max_id;
	}

	float NodeStatisticsHist::GetMaxProbability() const
	{
		return GetProbability( GetMaxBinId() );
	}

	vector<float> NodeStatisticsHist::GetAllProbability() const
	{
		vector<float> scores = bins;
		for(size_t i=0; i<scores.size(); i++)
			scores[i] /= SampleNum();

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

	void NodeStatisticsHist::AddSamples(const Mat& labels, const vector<int>& label_ids)
	{
		sampleCount += label_ids.size();
		for(size_t i=0; i<label_ids.size(); i++)
		{
			bins[labels.at<int>(label_ids[i])]++;
		}
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

	float DecisionTree::easy_sample_ratio = 0;
	bool DecisionTree::ifEasy = false;

	DecisionTree::DecisionTree(void)
	{
		useFastFeature = false;
	}

	bool DecisionTree::TrainTree(const Mat& samps, const Mat& labels, const DTreeTrainingParams& train_params)
	{
		if(samps.rows != labels.rows || labels.depth() != CV_32S )
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
		DecisionTreeNode rootNode(max_label+1);
		rootNode.nodeId = 0;
		rootNode.nodeLevel = 1;
		rootNode.samp_ids.clear();
		for(int i=0; i<labels.rows; i++)
			rootNode.samp_ids.push_back(i);
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
			// check if leaf
			if(cur_node.isLeaf)
				continue;

			// randomly choose feature and threshold
			AxisFeature best_feature;
			float best_th = 0;
			double maxGain = 0;
			for(int feat_id=0; feat_id<train_params.feature_num; feat_id++)
			{
				AxisFeature cur_feat;
				if(useFastFeature)
				{
					if(cur_node.nodeLevel < train_params.fastLevel)
					{
						// use only fast feature
						int rand_id = rand() % train_params.fast_feats_dims.size();
						cur_feat.axisIdx = train_params.fast_feats_dims[rand_id];
					}
					else
					{
						// use all features
						int rand_id = rand() % samps.cols;
						cur_feat.axisIdx = rand_id;
					}
				}
				else
				{
					// use all feature channels
					int rand_id = rand() % samps.cols;
					cur_feat.axisIdx = rand_id;
				}
				

				// apply to all samples
				vector<float> responses(cur_node.samp_ids.size());
				for(size_t i=0; i<cur_node.samp_ids.size(); i++)
					responses[i] = cur_feat.GetFeatureResponse(samps.row(cur_node.samp_ids[i]));

				// choose threshold
				vector<float> random_ths;
				if(responses.size() < train_params.th_num) random_ths = responses;
				else
				{
					for(int i=0; i<train_params.th_num; i++)
					{
						int th_id = rand() % responses.size();
						float cur_th = responses[th_id];
						random_ths.push_back(cur_th);
					}
				}

				// test each th
				for(int i=0; i<random_ths.size(); i++)
				{
					float cur_th = random_ths[i];

					// do separation
					NodeStatisticsHist leftHist(max_label+1);
					NodeStatisticsHist rightHist(max_label+1);
					for(size_t j=0; j<cur_node.samp_ids.size(); j++)
					{
						int cur_label = labels.at<int>(cur_node.samp_ids[j]);
						if(responses[j] < cur_th)
							leftHist.AddSample(cur_label);
						else
							rightHist.AddSample(cur_label);
					}

					// compute information gain
					double ig = NodeStatisticsHist::ComputeInformationGain(cur_node.trainingDataStatistics,
						leftHist, rightHist);
					if(ig > maxGain)
					{
						maxGain = ig;
						best_feature = cur_feat;
						best_th = cur_th;
					}
				}
			}

			// update current node
			cur_node.feature = best_feature;
			cur_node.th = best_th;

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
				float response = cur_node.feature.GetFeatureResponse(samps.row(cur_samp_id));
				if(response < cur_node.th)
				{
					leftNode.samp_ids.push_back(cur_samp_id);
					leftNode.trainingDataStatistics.AddSample(labels.at<int>(cur_samp_id));
				}
				else
				{
					rightNode.samp_ids.push_back(cur_samp_id);
					rightNode.trainingDataStatistics.AddSample(labels.at<int>(cur_samp_id));
				}
			}

			// check if leaf
			if(leftNode.trainingDataStatistics.GetMaxProbability() == 1 || leftNode.nodeLevel == train_params.MaxLevel)
				leftNode.isLeaf = true;
			if(rightNode.trainingDataStatistics.GetMaxProbability() == 1 || rightNode.nodeLevel == train_params.MaxLevel)
				rightNode.isLeaf = true;

			// add nodes to tree
			cout<<leftNode.nodeId<<" "<<rightNode.nodeId<<endl;
			nodes[leftNode.nodeId] = leftNode;
			nodes[rightNode.nodeId] = rightNode;
			toTrainNodes.push(leftNode.nodeId);
			toTrainNodes.push(rightNode.nodeId);
		}

		cout<<"Finish training."<<endl;
	}
		
	int DecisionTree::Predict(const Mat& samp, vector<float>& all_scores)
	{
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

			float response = cur_node.feature.GetFeatureResponse(samp);
			if(response < cur_node.th)
				cur_node_id = cur_node.nodeId*2 + 1;
			else
				cur_node_id = cur_node.nodeId*2 + 2;
		}
	}

	bool DecisionTree::Save(const string savefile)
	{
		ofstream out(savefile.c_str());
		if( !out.good() )
		{
			cerr<<"Can't open save file for dtree."<<endl;
			return false;
		}

		for(map<int, DecisionTreeNode>::iterator pi=nodes.begin(); pi!=nodes.end(); pi++)
		{
			// node id
			out<<pi->first<<" "<<pi->second.nodeLevel<<" "<<pi->second.isLeaf<<" "
				<<pi->second.feature.axisIdx<<" "<<pi->second.th<<" "
				<<pi->second.trainingDataStatistics.sampleCount<<" "<<pi->second.trainingDataStatistics.bins.size()<<" ";
			for(size_t i=0; i<pi->second.trainingDataStatistics.bins.size(); i++)
				out<<(i==0? "":" ")<<pi->second.trainingDataStatistics.bins[i];
			out<<endl;
		}

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

		int nodeId;
		nodes.clear();
		while(in>>nodeId)
		{
			DecisionTreeNode cur_node;
			cur_node.nodeId = nodeId;
			in>>cur_node.nodeLevel>>cur_node.isLeaf>>cur_node.feature.axisIdx>>cur_node.th;
			int binSize;
			in>>cur_node.trainingDataStatistics.sampleCount>>binSize;
			cur_node.trainingDataStatistics.bins.resize(binSize);
			for(size_t i=0; i<binSize; i++)
				in>>cur_node.trainingDataStatistics.bins[i];

			nodes[nodeId] = cur_node;
		}

		return true;
	}

}

