#include "ObjectRanker.h"


namespace visualsearch
{
	namespace processors
	{
		namespace attention
		{
			ObjectRanker::ObjectRanker(void)
			{
				features::color::ColorFeatParams cparams;
				cparams.feat_type = features::color::COLOR_FEAT_MEAN;
				cparams.histParams.color_space = features::color::COLOR_LAB;
				colordesc.Init(cparams);

				features::DepthFeatParams dparams;
				dparams.dtype = DEPTH_FEAT_HIST;
			}

			//////////////////////////////////////////////////////////////////////////

			bool ObjectRanker::RankSegments(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids)
			{
				if(rtype == SEG_RANK_CC)
					return RankSegmentsByCC(cimg, sps, orded_sp_ids);
				if(rtype == SEG_RANK_SALIENCY)
					return RankSegmentsBySaliency(cimg, dmap, sps, orded_sp_ids);

				return true;
			}

			bool ObjectRanker::RankWindowsBySaliency(const Mat& cimg, vector<ImgWin>& wins, vector<int>& ordered_win_ids)
			{
				map<float, int, greater<float>> win_scores;
				vector<Mat> feats;
				ComputeWindowRankFeatures(cimg, Mat(), wins, feats);
				for (size_t i=0; i<wins.size(); i++)
				{
					wins[i].score = (float)sum(feats[i]).val[0] / feats[i].cols;
					win_scores[wins[i].score] = i;
				}

				ordered_win_ids.clear();
				ordered_win_ids.reserve(wins.size());
				for (map<float, int, greater<float>>::const_iterator pi=win_scores.begin();
					pi!=win_scores.end(); pi++)
				{
					ordered_win_ids.push_back(pi->second);
				}

				return true;
			}

			// use saliency map to find the best salient segments
			// multi-scale contrast is used to filter  
			bool ObjectRanker::RankSegmentsBySaliency(const Mat& cimg, const Mat& dmap, vector<SuperPixel>& sps, vector<int>& orded_sp_ids)
			{
				bool use_comp = true;
				map<float, int, greater<float>> sp_scores;
				if(use_comp) {
					// compute composition cost for each superpixel
					ImageSegmentor segmentor;
					segmentor.m_dThresholdK = 30.f;
					segmentor.seg_type_ = OVER_SEG_GRAPH;
					cout<<"seg num "<<segmentor.DoSegmentation(cimg)<<endl;
					imshow("baseseg", segmentor.m_segImg);
					waitKey(10);
					for(size_t i=0; i<segmentor.superPixels.size(); i++) 
						segprocessor.ExtractBasicSegmentFeatures(segmentor.superPixels[i], cimg, dmap);

					sal_comp_.Init(SAL_COLOR, cimg, dmap, segmentor.superPixels);
					for (size_t i=0; i<sps.size(); i++) {
						float score = sal_comp_.Compose(sps[i]);// * ((float)sps[i].area / contourArea(sps[i].convex_hull));
						sp_scores[score] = i;
					}
				}
				else {
					// compute saliency map
					Mat sal_map;
					salcomputer.ComputeSaliencyMap(cimg, SAL_HC, sal_map);
					imshow("salmap", sal_map);

					// compute saliency score for each superpixel
					float context_ratios[3] = {1.0, 1.2, 1.3};
					for (size_t i=0; i<sps.size(); i++)
					{
						float mean_obj_score = (float)cv::mean(sal_map, sps[i].mask).val[0];
						float sum_obj_score = mean_obj_score * sps[i].area;
						float sum_context_score = 0;
						float context_scores[3];
						for(int k=0; k<3; k++) { 
							ImgWin context_win = tools::ToolFactory::GetContextWin(cimg.cols, cimg.rows, sps[i].box, context_ratios[k]);
							//cout<<context_win<<endl;
							context_scores[k] = (cv::mean(sal_map(context_win)).val[0]*context_win.area() - sum_obj_score) / (context_win.area()-sps[i].area);
							sum_context_score += context_scores[k];
						}

						float total_diff = fabs(sum_context_score - 3*mean_obj_score) / 3;
						total_diff = total_diff * mean_obj_score;
						sp_scores[total_diff] = i;
					}
				}

				orded_sp_ids.clear();
				orded_sp_ids.reserve(sp_scores.size());
				for (auto pi=sp_scores.begin(); pi!=sp_scores.end(); pi++) {
					orded_sp_ids.push_back(pi->second);
				}

				return true;
			}

			bool ObjectRanker::RankSegmentsByCC(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids)
			{
				map<float, int, greater<float>> sp_scores;
				for (size_t i=0; i<sps.size(); i++)
				{
					float curscore = cs_contraster.ComputeContrast(cimg, Mat(), sps[i], FEAT_TYPE_COLOR, 1.2);
					sp_scores[curscore] = i;
				}

				orded_sp_ids.clear();
				orded_sp_ids.reserve(sp_scores.size());
				for (map<float, int, greater<float>>::const_iterator pi=sp_scores.begin();
					pi!=sp_scores.end(); pi++)
				{
					orded_sp_ids.push_back(pi->second);
				}

				return true;
			}

			bool ObjectRanker::ComputeSegmentRankFeature(const Mat& cimg, const Mat& dmap, SuperPixel& sp, Mat& feat)
			{
				vector<float> vals;
				// geometric features
				segprocessor.ExtractBasicSegmentFeatures(sp, cimg, dmap);
				//vector<SuperPixel> sps;
				//sps.push_back(sp);
				//img_vis_.DrawShapes(cimg, sps);
				//waitKey(0);
				vals.push_back((float)sp.area / (sp.mask.rows*sp.mask.cols));	// area percentage
				//vals.push_back(sp.isConvex);
				vals.push_back((float)sp.area / sp.box.area());	// segment / box ratio
				vals.push_back((float)sp.perimeter / (2*(sp.box.width+sp.box.height)));	// segment perimeter / box perimeter ratio
				vals.push_back((float)sp.centroid.x / cimg.cols);
				vals.push_back((float)sp.centroid.y / cimg.rows);	// normalized position in image
				vals.push_back((float)sp.box.width);
				vals.push_back((float)sp.box.height);

				// saliency features
				vals.push_back( cs_contraster.ComputeContrast(cimg, Mat(), sp, FEAT_TYPE_COLOR, 1.0) );
				vals.push_back( cs_contraster.ComputeContrast(cimg, Mat(), sp, FEAT_TYPE_COLOR, 1.2) );
				vals.push_back( cs_contraster.ComputeContrast(cimg, Mat(), sp, FEAT_TYPE_COLOR, 1.5) );
				// TODO: use saliency composition cost

				// depth: depth contrast
				if( !dmap.empty() ) {
					vals.push_back( cs_contraster.ComputeContrast(cimg, dmap, sp, FEAT_TYPE_DEPTH, 1.0) );
					vals.push_back( cs_contraster.ComputeContrast(cimg, dmap, sp, FEAT_TYPE_DEPTH, 1.2) );
					vals.push_back( cs_contraster.ComputeContrast(cimg, dmap, sp, FEAT_TYPE_DEPTH, 1.5) );
					Mat meand;
					depth_desc_.ComputeMeanDepth(dmap, meand, sp.mask);
					vals.push_back( meand.at<float>(0) );
					// TODO: use depth composition cost

					// 3D
				}

				// convert to mat
				feat.create(1, vals.size(), CV_32F);
				for (size_t i=0; i<vals.size(); i++) feat.at<float>(i) = vals[i];

				return true;
			}

			bool ObjectRanker::ComputeWindowRankFeatures(const Mat& cimg, const Mat& dmap, vector<ImgWin>& wins, vector<Mat>& feats)
			{
				feats.clear();
				feats.resize(wins.size());
				// geometric features
				//vals.push_back(win.area() / (cimg.rows*cimg.cols));		// area percentage
				//vals.push_back(mean(dmap(win)).val[0]);					// mean depth
				//vals.push_back(win.width*1.0f / win.height);			// window ratio
				//vals.push_back((win.x+win.width/2)*1.0f / cimg.cols);
				//vals.push_back((win.y+win.height/2)*1.0f / cimg.rows);	// relative position in image

				// saliency features
				vector<Mat> salmaps(1);
				//salcomputer.ComputeSaliencyMap(cimg, SAL_FT, salmaps[0]);
				//salcomputer.ComputeSaliencyMap(cimg, SAL_SR, salmaps[1]);
				salcomputer.ComputeSaliencyMap(cimg, SAL_HC, salmaps[0]);
				//salcomputer.ComputeSaliencyMap(cimg, SAL_LC, salmaps[3]);

				for (size_t i=0; i<wins.size(); i++)
				{
					vector<float> vals;
					for (size_t j=0; j<salmaps.size(); j++)
					{
						//cout<<salmaps[j].rows<<" "<<salmaps[j].cols<<endl;
						vals.push_back(mean(salmaps[j](wins[i])).val[0]);		// mean ft sal
					}
					
					// convert to mat
					feats[i].create(1, vals.size(), CV_32F);
					for (size_t k=0; k<vals.size(); k++) feats[i].at<float>(k) = vals[k];
				}

				return true;
			}

			//////////////////////////////////////////////////////////////////////////

			bool ObjectRanker::LearnObjectPredictor()
			{
				Mat pred_scores(rank_test_data.rows, 2, CV_32F);
				pred_scores.setTo(0);
				visualsearch::learners::LearnerTools learn_tool;

				// train svm
				CvSVM model;
				SVMParams params;
				rank_train_label = rank_train_label * 2 - 1;	// transform label to be -1 and 1
				model.train_auto(rank_train_data, rank_train_label, Mat(), Mat(), params);

				// save
				model.save("svm.model");
				
				// evaluation
				for (int r=0; r<rank_test_data.rows; r++) {
					float res = model.predict(rank_test_data.row(r), true);
					if(res > 0) pred_scores.at<float>(r, 1) = res;
					else		pred_scores.at<float>(r, 0) = -res;
				}
				cout<<pred_scores<<endl;
				float acc = learn_tool.ClassificationAccuracy(rank_test_label, pred_scores, 1);
				cout<<"Accuracy: "<<acc<<endl;

				return true;


				// dtrees
				learners::trees::RandomForest rforest;
				learners::trees::RForestTrainingParams rfparams;

				rfparams.tree_params.feat_type = learners::trees::DTREE_FEAT_LINEAR;
				rfparams.num_trees = 5;
				rfparams.tree_params.min_samp_num = 10;
				rfparams.tree_params.MaxLevel = 8;
				rfparams.tree_params.feature_num = 200;
				rfparams.tree_params.th_num = 50;

				rforest.Train(rank_train_data, rank_train_label, rfparams);
				ofstream out("forest_ranker.dat");
				rforest.Save(out);
				//rforest.EvaluateRandomForest(rank_test_data, rank_test_label, 2);

				// evaluate performance
				for (int r=0; r<rank_test_data.rows; r++)
				{
					vector<double> scores;
					rforest.ForestPredict(rank_test_data.row(r), scores);
					for(size_t i=0; i<scores.size(); i++) pred_scores.at<float>(r, i) = (float)scores[i];
					/*float res = model.predict(rank_test_data.row(r));
					if(res > 0) 
					pred_scores.at<float>(1) = res;
					else 
					pred_scores.at<float>(0) = -res;*/
				}
				acc = learn_tool.ClassificationAccuracy(rank_test_label, pred_scores, 1);
				cout<<"Accuracy: "<<acc<<endl;

				return true;
			}

			bool ObjectRanker::LearnObjectWindowPredictor()
			{
				string temp_dir = "E:\\Results\\objectness\\";	// save intermediate results
				char str[50];

				ImageSegmentor imgsegmentor;
				vector<float> seg_ths(3);
				seg_ths[0] = 50;
				seg_ths[1] = 100;
				seg_ths[2] = 200;

				// generate training samples from nyudata
				NYUDepth2DataMan nyuman;
				FileInfos imgfiles, dmapfiles;
				nyuman.GetImageList(imgfiles);
				nyuman.GetDepthmapList(imgfiles, dmapfiles);
				map<string, vector<Mat>> objmasks;
				imgfiles.erase(imgfiles.begin()+10, imgfiles.end());
				nyuman.LoadGTMasks(imgfiles, objmasks);

				// positive sample: object segments
				Mat possamps, negsamps;
				for(size_t i=0; i<imgfiles.size(); i++)
				{
					Mat cimg = imread(imgfiles[i].filepath);
					Mat dmap;
					nyuman.LoadDepthData(dmapfiles[i].filepath, dmap);

					const vector<Mat> masks = objmasks[imgfiles[i].filename];
					vector<ImgWin> poswins;
					for (size_t k=0; k<masks.size(); k++)
					{
						SuperPixel cursegment;
						cursegment.mask = masks[k];
						segprocessor.ExtractBasicSegmentFeatures(cursegment, cimg, dmap);
						sprintf_s(str, "%d_posseg_%d.jpg", i, k);
						//imwrite(temp_dir + string(str), cursegment.mask*255);
						poswins.push_back(ImgWin(cursegment.box.x, cursegment.box.y, cursegment.box.width, cursegment.box.height));
						vector<ImgWin> wins;
						wins.push_back(poswins[poswins.size()-1]);
						vector<Mat> feats;
						ComputeWindowRankFeatures(cimg, dmap, wins, feats);
						possamps.push_back(feats[0]);
					}

					// negative samples: random window don't overlap with object windows
					int negnum = 0;
					vector<ImgWin> negwins;
					while(negnum < masks.size())
					{
						// generate random window
						ImgWin selwin;
						selwin.x = rand() % cimg.cols;
						selwin.y = rand() % cimg.rows;
						selwin.width = rand() % (cimg.cols-selwin.x-1);
						selwin.height = rand() % (cimg.rows-selwin.y-1);

						// test against all object windows
						bool isvalid = true;
						for(size_t k=0; k<poswins.size(); k++)
						{
							Rect intersectWin = selwin & poswins[k];
							if(intersectWin.area()*1.0f / poswins[k].area() > 0.3f)
							{ isvalid = false; break; }
						}
						// pass all tests
						if(isvalid)
						{
							sprintf_s(str, "%d_negseg_%d.jpg", i, negnum);
							//imwrite(temp_dir + string(str), tsps[sel_id].mask*255);
							negwins.push_back(selwin);
							vector<ImgWin> wins;
							wins.push_back(selwin);
							vector<Mat> feats;
							ComputeWindowRankFeatures(cimg, dmap, wins, feats);
							negsamps.push_back(feats[0]);
							negnum++;
						}
					}

					cout<<"Finished "<<i<<"/"<<imgfiles.size()<<" image"<<endl;
				}

				// train svm
				CvSVM model;
				Mat responses(1, possamps.rows+negsamps.rows, CV_32S);
				for(int r=0; r<possamps.rows; r++) responses.at<int>(r) = 1;
				for(int r=possamps.rows; r<responses.cols; r++) responses.at<int>(r) = -1;
				Mat allsamps;
				allsamps.push_back(possamps);
				allsamps.push_back(negsamps);

				SVMParams params;	
				model.train_auto(allsamps, responses, Mat(), Mat(), params);

				// save
				model.save("svm.model");

				// training performance
				float pos_corr = 0;
				for (int r=0; r<possamps.rows; r++)
				{
					float res = model.predict(possamps.row(r));
					if(res > 0)
						pos_corr++;
				}
				float neg_corr = 0;
				for (int r=0; r<negsamps.rows; r++)
				{
					float res = model.predict(negsamps.row(r));
					if(res < 0)
						neg_corr++;
				}
				cout<<"Pos accu: "<<pos_corr / possamps.rows<<endl;
				cout<<"Neg accu: "<<neg_corr / negsamps.rows<<endl;
				cout<<"total accu: "<<(pos_corr+neg_corr) / (possamps.rows + negsamps.rows)<<endl;

				return true;

			}

			// positive samples: gt object and its variants
			// negative samples: random segments having low overlap with gt objects
			bool ObjectRanker::PrepareRankTrainData(DatasetName dbs)
			{
				test_ = false;
				string temp_dir = "E:\\Results\\objectness\\";	// save intermediate results
				char str[50];

				ImageSegmentor imgsegmentor;
				std::vector<float> seg_ths(3);
				seg_ths[0] = 50;
				seg_ths[1] = 100;
				seg_ths[2] = 200;

				// generate training samples from given database
				FileInfos imgfiles, dmapfiles;
				DataManagerInterface* db_man = NULL;
				map<string, vector<Mat>> objmasks;
				if( (dbs & DB_NYU2_RGBD) != 0 )
					db_man = new NYUDepth2DataMan();
				if( (dbs & DB_SALIENCY_RGBD) != 0 )
					db_man = new RGBDECCV14();

				db_man->GetImageList(imgfiles);
				db_man->GetDepthmapList(imgfiles, dmapfiles);
				imgfiles.erase(imgfiles.begin()+400, imgfiles.end());
				db_man->LoadGTMasks(imgfiles, objmasks);

				cout<<"Generating training samples..."<<endl;
				Mat possamps, negsamps;
				for(size_t i=0; i<imgfiles.size(); i++)
				{
					Mat cimg = imread(imgfiles[i].filepath);
					Size newsz;
					tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
					resize(cimg, cimg, newsz);
					Mat dmap;
					db_man->LoadDepthData(dmapfiles[i].filepath, dmap);
					resize(dmap, dmap, newsz);

					imshow("color", cimg);
					img_vis_.DrawFloatImg("dmap", dmap, Mat());
					
					vector<Mat> masks = objmasks[imgfiles[i].filename];
					// positive sample: object segments
					for (size_t k=0; k<masks.size(); k++) {
						resize(masks[k], masks[k], newsz);
						SuperPixel cursegment;
						cursegment.mask = masks[k];
						//resize(cursegment.mask, cursegment.mask, newsz);
						sprintf_s(str, "%d_posseg_%d.jpg", i, k);
						//imwrite(temp_dir + string(str), cursegment.mask*255);
						if(test_) {
							debug_img_.release();
							cimg.copyTo(debug_img_, cursegment.mask);
							imshow("pos", debug_img_);
							waitKey(0);
						}

						Mat curposfeat;
						ComputeSegmentRankFeature(cimg, dmap, cursegment, curposfeat);
						possamps.push_back(curposfeat);
					}

					// negative samples: random segments don't overlap with objects
					for (size_t k=0; k<seg_ths.size(); k++) {
						imgsegmentor.m_dThresholdK = seg_ths[k];
						imgsegmentor.DoSegmentation(cimg);
						vector<SuperPixel>& tsps = imgsegmentor.superPixels;

						// randomly select samples from every segment level
						random_shuffle(tsps.begin(), tsps.end());
						int sumnum = 0;
						for (size_t i=0; i<tsps.size(); i++) {
							if(tsps[i].area < cimg.rows*cimg.cols*0.05) continue;
							// check if have large overlap with one of the objects
							bool isvalid = true;
							for (size_t j=0; j<masks.size(); j++) {
								Mat curmask = masks[j];
								//resize(masks[j], curmask, newsz);
								Mat intersectMask = curmask & tsps[i].mask;
								if( countNonZero(intersectMask) / countNonZero(curmask) > 0.5 ) {
									isvalid = false;
									break;
								}
							}
							// pass all tests
							sprintf_s(str, "%d_negseg_%d.jpg", i, sumnum);
							//imwrite(temp_dir + string(str), tsps[sel_id].mask*255);
							if(test_) {
								debug_img_.release();
								cimg.copyTo(debug_img_, tsps[i].mask);
								imshow("neg", debug_img_);
								waitKey(0);
							}

							Mat curnegfeat;
							ComputeSegmentRankFeature(cimg, dmap, tsps[i], curnegfeat);
							negsamps.push_back(curnegfeat);
							sumnum++;
							if(sumnum == masks.size()*2) break;
						}
					}

					cout<<"Finished "<<i<<"/"<<imgfiles.size()<<" image"<<endl;
				}

				// divide into train and test data with 3:7
				rank_train_data.release();
				rank_test_data.release();
				rank_train_label.release();
				rank_test_label.release();
				for (int r=0; r<possamps.rows; r++)
				{
					if(r<possamps.rows*0.8) {
						rank_train_data.push_back(possamps.row(r));
						rank_train_label.push_back(1);
					}
					else {
						rank_test_data.push_back(possamps.row(r));
						rank_test_label.push_back(1);
					}
				}
				for (int r=0; r<negsamps.rows; r++)
				{
					if(r<negsamps.rows*0.8) {
						rank_train_data.push_back(negsamps.row(r));
						rank_train_label.push_back(0);
					}
					else {
						rank_test_data.push_back(negsamps.row(r));
						rank_test_label.push_back(0);
					}
				}

				cout<<"Rank training data ready."<<endl;

				delete db_man;
				db_man = NULL;

				return true;
			}
		}
	}
	
}



