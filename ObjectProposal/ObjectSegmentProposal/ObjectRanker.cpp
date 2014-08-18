#include "ObjectRanker.h"

namespace visualsearch
{
	namespace processors
	{
		namespace attention
		{
			ObjectRanker::ObjectRanker(void)
			{
				features::ColorFeatParams cparams;
				cparams.feat_type = features::COLOR_FEAT_MEAN;
				cparams.histParams.color_space = features::COLOR_LAB;
				colordesc.Init(cparams);



			}

			//////////////////////////////////////////////////////////////////////////

			bool ObjectRanker::RankSegments(const Mat& cimg, const Mat& dmap, const vector<SuperPixel>& sps, SegmentRankType rtype, vector<int>& orded_sp_ids)
			{
				if(rtype == SEG_RANK_CC)
					return RankSegmentsByCC(cimg, sps, orded_sp_ids);

				return true;
			}

			bool ObjectRanker::RankSegmentsByCC(const Mat& cimg, const vector<SuperPixel>& sps, vector<int>& orded_sp_ids)
			{
				map<float, int, greater<float>> sp_scores;
				for (size_t i=0; i<sps.size(); i++)
				{
					float curscore = ComputeCenterSurroundColorContrast(cimg, sps[i]);
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
				vals.push_back(sp.area / (sp.mask.rows*sp.mask.cols));	// area percentage
				vals.push_back(sp.isConvex);
				vals.push_back(sp.meanDepth);
				vals.push_back(sp.area / sp.box.area());	// segment /box ratio
				vals.push_back(sp.perimeter / (2*(sp.box.width+sp.box.height)));	// segment perimeter / box perimeter ratio
				vals.push_back(sp.centroid.x / cimg.cols);
				vals.push_back(sp.centroid.y / cimg.rows);	// relative position in image

				// saliency features


				// convert to mat
				feat.create(1, vals.size(), CV_32F);
				for (size_t i=0; i<vals.size(); i++) feat.at<float>(i) = vals[i];

				return true;
			}

			float ObjectRanker::ComputeCenterSurroundColorContrast(const Mat& cimg, const SuperPixel& sp)
			{
				// ignore object bigger than half the image
				if(sp.box.area() > cimg.rows*cimg.cols*0.5f)
					return 0;

				// context window
				ImgWin spbox(sp.box.x, sp.box.y, sp.box.width, sp.box.height);
				ImgWin contextWin = tools::ToolFactory::GetContextWin(cimg.cols, cimg.rows, spbox, 1.5);
				Mat contextMask(cimg.rows, cimg.cols, CV_8U);
				contextMask.setTo(0);
				contextMask(contextWin).setTo(1);
				contextMask.setTo(0, sp.mask);

				// compute color descriptors
				Mat contextColor;
				Mat spColor;
				colordesc.Compute(cimg, contextColor, contextMask);
				colordesc.Compute(cimg, spColor, sp.mask);

				float score = norm(contextColor, spColor, NORM_L2);
				return score;
			}

			bool ObjectRanker::LearnObjectPredictorFromNYUDepth()
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
				nyuman.GetDepthmapList(dmapfiles);
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
					for (size_t k=0; k<masks.size(); k++)
					{
						SuperPixel cursegment;
						cursegment.mask = masks[k];
						sprintf_s(str, "%d_posseg_%d.jpg", i, k);
						//imwrite(temp_dir + string(str), cursegment.mask*255);
						Mat curposfeat;
						ComputeSegmentRankFeature(cimg, dmap, cursegment, curposfeat);
						possamps.push_back(curposfeat);
					}

					// negative samples: random segments don't overlap with objects
					for (size_t k=0; k<seg_ths.size(); k++)
					{
						imgsegmentor.m_dThresholdK = seg_ths[k];
						imgsegmentor.DoSegmentation(cimg);
						vector<SuperPixel>& tsps = imgsegmentor.superPixels;
						// randomly select 3x samples for every segment level
						int sumnum = 0;
						while(sumnum < masks.size())
						{
							int sel_id = rand() % imgsegmentor.superPixels.size();
							bool isvalid = true;
							for (size_t j=0; j<masks.size(); j++)
							{
								Mat intersectMask = masks[j] & tsps[sel_id].mask;
								if( countNonZero(intersectMask) / countNonZero(masks[j]) > 0.3 )
								{
									isvalid = false;
									break;
								}
							}
							// pass all tests
							sprintf_s(str, "%d_negseg_%d.jpg", i, sumnum);
							//imwrite(temp_dir + string(str), tsps[sel_id].mask*255);
							Mat curnegfeat;
							ComputeSegmentRankFeature(cimg, dmap, tsps[sel_id], curnegfeat);
							negsamps.push_back(curnegfeat);
							sumnum++;
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
		}
	}
	
}



