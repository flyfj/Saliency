#include "RandomForest.h"


namespace learners
{

		bool RandomForest::Train(const cv::Mat& samps, const cv::Mat& labels, const RForestTrainingParams& params)
		{
				// train each tree
				tree_collection.clear();
				tree_collection.resize(params.num_trees);

				std::vector<int> samp_ids;
				for(int i=0; i<samps.rows; i++)
						samp_ids.push_back(i);

				float subset_percentage = 0.7f;
				int subset_size = (params.split_disjoint? samp_ids.size() / params.num_trees: samp_ids.size()*subset_percentage);
				// get corresponding sample ids for each class
				vector<vector<int>> class_labels;
				tools::ToolFactory::SplitClassSamples(labels, class_labels, true);

				for(int i=0; i<params.num_trees; i++)
				{
						// select random subset of training samples
						cv::Mat cur_samps(0, 0, CV_32F);
						cv::Mat cur_labels(0, 0, CV_32S);

						int count = 0;
						if( !params.split_disjoint )
						{
								// shuffle every class
							  for(int k=0; k<class_labels.size(); k++)
								{
										std::random_shuffle(class_labels[k].begin(), class_labels[k].end());
										// pick the first subset
										for(size_t r=0; r<class_labels[k].size()*subset_percentage; r++)
										{
												cur_samps.push_back( samps.row(class_labels[k][r]) );
												cur_labels.push_back(k);
												count++;
										}
								}
						}
						else
						{
								// pick corresponding part from each class
								for(int k=0; k<class_labels.size(); k++)
								{
										int part_len = class_labels[k].size() / params.num_trees;
										for(size_t id=0; id<part_len; id++)
										{
												cur_samps.push_back( samps.row(class_labels[k][i*part_len+id]) );
												cur_labels.push_back(k);
												count++;
										}
								}
						}

						cout<<cur_labels.depth()<<endl;
						// train each tree
						if( !tree_collection[i].TrainTree(cur_samps, cur_labels, params.tree_params) )
						{
								std::cerr<<i<<"th tree not trained"<<std::endl;
								return false;
						}
				}

				m_ifTrained = true;

				return true;
		}

		bool RandomForest::Train(const std::vector<cv::Mat>& samp_imgs, const cv::Mat& labels, const RForestTrainingParams& params)
		{
				// train each tree
				tree_collection.clear();
				tree_collection.resize(params.num_trees);

				float subset_percentage = 0.7f;
				int subset_size = (params.split_disjoint? samp_imgs.size() / params.num_trees: samp_imgs.size()*subset_percentage);
				// get corresponding sample ids for each class
				vector<vector<int>> class_labels;
				tools::ToolFactory::SplitClassSamples(labels, class_labels, true);

				for(int i=0; i<params.num_trees; i++)
				{
						// select random subset of training samples
						vector<cv::Mat> cur_samps;
						cv::Mat cur_labels(0, 0, CV_32S);

						int count = 0;
						if( !params.split_disjoint )
						{
								// shuffle every class
							  for(int k=0; k<class_labels.size(); k++)
								{
										std::random_shuffle(class_labels[k].begin(), class_labels[k].end());
										// pick the first subset
										for(size_t r=0; r<class_labels[k].size()*subset_percentage; r++)
										{
												cur_samps.push_back( samp_imgs[class_labels[k][r]] );
												cur_labels.push_back(k);
												count++;
										}
								}
						}
						else
						{
								// pick corresponding part from each class
								for(int k=0; k<class_labels.size(); k++)
								{
										int part_len = class_labels[k].size() / params.num_trees;
										for(size_t id=0; id<part_len; id++)
										{
												cur_samps.push_back( samp_imgs[class_labels[k][i*part_len+id]] );
												cur_labels.push_back(k);
												count++;
										}
								}
						}

						//cout<<cur_labels.depth()<<endl;
						
						// train each tree
						if( !tree_collection[i].TrainTree(cur_samps, cur_labels, params.tree_params) )
						{
								std::cerr<<i<<"th tree not trained"<<std::endl;
								return false;
						}
						cout<<"Finish training tree: "<<i<<endl<<endl;
				}

				m_ifTrained = true;

				return true;
		}

		int RandomForest::ForestPredict(const cv::Mat& samp, std::vector<double>& all_scores)
		{
			if(!m_ifTrained)
				return -1;

			vector<double> temp_prob;
			all_scores.clear();

			for(size_t i=0; i<tree_collection.size(); i++)
			{
				tree_collection[i].Predict(samp, temp_prob);
				if(all_scores.empty()) all_scores.resize(temp_prob.size(), 0);
				std::transform(all_scores.begin(), all_scores.end(), temp_prob.begin(), all_scores.begin(), plus<double>());
			}

			int max_label = -1;
			double max_prob = 0;
			for(size_t i=0; i<all_scores.size(); i++)
			{
				all_scores[i] /= tree_collection.size();
				if(all_scores[i] > max_prob)
				{
					max_prob = all_scores[i];
					max_label = i;
				}
			}

			return max_label;
		}

		int RandomForest::PredictImage(const cv::Mat& img, std::vector<double>& all_scores)
		{
				if(!m_ifTrained)
						return -1;
				
				vector<double> temp_prob;
				all_scores.clear();

				for(size_t i=0; i<tree_collection.size(); i++)
				{
						tree_collection[i].PredictImage(img, temp_prob);
						if(all_scores.empty()) all_scores.resize(temp_prob.size(), 0);
						std::transform(all_scores.begin(), all_scores.end(), temp_prob.begin(), all_scores.begin(), plus<double>());
				}

				int max_label = -1;
				double max_prob = 0;
				for(size_t i=0; i<all_scores.size(); i++)
				{
						all_scores[i] /= tree_collection.size();
						if(all_scores[i] > max_prob)
						{
								max_prob = all_scores[i];
								max_label = i;
						}
				}

				return max_label;
		}

		bool RandomForest::EvaluateRandomForest(const cv::Mat& test_samps, const cv::Mat& test_labels, int nClass)
		{
				if(!m_ifTrained || test_samps.rows != test_labels.rows)
					return false;

					// compute confusion matrix
					cv::Mat confusionMat(nClass, nClass, CV_64F);
					confusionMat.setTo(0);
					vector<double> all_scores;
					for(int r=0; r<test_samps.rows; r++)
					{
							int pred_label = ForestPredict(test_samps.row(r), all_scores);
							if(test_labels.at<int>(r) == 17)
									cout<<pred_label<<endl;

							confusionMat.at<double>(test_labels.at<int>(r), pred_label)++;
					}

					// normalize
					double avg_acc = 0;
					for(int r=0; r<confusionMat.rows; r++)
					{
			  			cv::normalize(confusionMat.row(r), confusionMat.row(r), 1, 0, cv::NORM_L1);
							avg_acc += confusionMat.at<double>(r,r);

							cout<<"Accuracy for class "<<r<<" : "<<confusionMat.at<double>(r,r)<<endl;;
					}

					avg_acc /= confusionMat.rows;
					cout<<"Average accuracy: "<<avg_acc<<endl;

					cout<<confusionMat<<endl;

					return true;
		}

		//////////////////////////////////////////////////////////////////////////

		bool RandomForest::Save(const std::string savefile)
		{
				ofstream out(savefile.c_str());
				if( !out.good() )
				{
					 cerr<<"Can't open save file for forest."<<endl;
					 return false;
				}

				// save each tree
				for(size_t i=0; i<tree_collection.size(); i++)
				{
						const DecisionTree& cur_tree = tree_collection[i];
						
						out<<cur_tree.max_tree_level<<" "<<cur_tree.nodes.size()<<endl;
						const std::map<int, DecisionTreeNode>& nodes = cur_tree.nodes;
						for(map<int, DecisionTreeNode>::const_iterator pi=nodes.begin(); pi!=cur_tree.nodes.end(); pi++)
						{
									// node id
									out<<pi->first<<" "<<pi->second.nodeLevel<<" "<<pi->second.isLeaf<<" "
											<<pi->second.pair_feature<<" "<<pi->second.th<<" "
											//<<pi->second.feature.GetAxisId()<<" "<<pi->second.th<<" "
										<<pi->second.trainingDataStatistics.sampleCount<<" "<<pi->second.trainingDataStatistics.bins.size()<<" ";
									for(size_t i=0; i<pi->second.trainingDataStatistics.bins.size(); i++)
										out<<(i==0? "":" ")<<pi->second.trainingDataStatistics.bins[i];
									out<<endl;
						}
				}

				std::cout<<"Forest saved to "<<savefile<<std::endl;

				return true;
		}

	  bool RandomForest::Load(const std::string loadfile)
		{
				ifstream in(loadfile.c_str());
				if(!in.good())
				{
					cerr<<"Can't open load file for dtree."<<endl;
					return false;
				}

				tree_collection.clear();

				int max_tree_level;
				int node_num;
				while(in>>max_tree_level>>node_num)
				{
						// read current tree
						DecisionTree cur_tree;
						cur_tree.max_tree_level = max_tree_level;
						// read nodes
						int nodeId;
						cur_tree.nodes.clear();
						for(int i=0; i<node_num; i++)
						{
								in>>nodeId;
								DecisionTreeNode cur_node;
								cur_node.nodeId = nodeId;
								int axis_id;
								cv::Point binIds;
								in>>cur_node.nodeLevel>>cur_node.isLeaf
										>>cur_node.pair_feature>>cur_node.th;
										//>>axis_id>>cur_node.th;
								//cur_node.pixel_feature = PixelComparisonFeature(binIds.x, binIds.y);
								//cur_node.axis_feature.SetAxisId(axis_id);
								int binSize;
								in>>cur_node.trainingDataStatistics.sampleCount>>binSize;
								cur_node.trainingDataStatistics.bins.resize(binSize);
								for(size_t i=0; i<binSize; i++)
										in>>cur_node.trainingDataStatistics.bins[i];
			
								cur_tree.nodes[nodeId] = cur_node;
						}

						cur_tree.m_ifTrained = true;
						tree_collection.push_back(cur_tree);
				}
		

				std::cout<<"Forest loaded from "<<loadfile<<std::endl;

				m_ifTrained = true;

				return true;
		}

		//////////////////////////////////////////////////////////////////////////

		void RandomForest::DrawPixelDecisionBoundary(int width, int height, int id)
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
							int pred_label = ForestPredict(samp, all_scores);
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

		void RandomForest::DrawPixelWeightMap(const cv::Mat& samps, const std::vector<double>& weights, int id)
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
}

