#include "ColorDescriptors.h"


namespace visualsearch
{
	namespace features
	{
		ColorDescriptors::ColorDescriptors(void)
		{
			MAX_IMG_DIM = 300;
		}

		bool ColorDescriptors::Init(const ColorFeatParams& cparams)
		{
			params = cparams;
			if(params.feat_type == COLOR_FEAT_HIST && params.histParams.usePalette)
				CreateFixedColorPalette(
				params.histParams.color_space, 
				cv::Point3d(params.histParams.hist_bin_nums[0], params.histParams.hist_bin_nums[1], params.histParams.hist_bin_nums[2]));

			return true;
		}

		bool ColorDescriptors::CreateFixedColorPalette(FeatColorSpace colorspace, cv::Point3d nums)
		{
			bool verbose = false;

			if(colorspace == COLOR_RGB)
			{
				// uniformly sample color from space
				vector<int> rvalues(nums.x);
				vector<int> gvalues(nums.y);
				vector<int> bvalues(nums.z);
				for(int i=0; i<nums.x; i++)
				{
					int step = 100 / nums.x;
					rvalues[i] = int(step * i + step/2 - 0);
				}
				for(int i=0; i<nums.y; i++)
				{
					int step = 255 / nums.y;
					gvalues[i] = int(step * i + step/2 - 127);
				}
				for(int i=0; i<nums.z; i++)
				{
					int step = 255 / nums.z;
					bvalues[i] = int(step * i + step/2 - 127);
				}

				int grid_num = nums.x * nums.y * nums.z;
				int grid_per_row = nums.x;	//(int)(sqrt(double(grid_num)));
				int grid_per_col = grid_num / grid_per_row;

				vector<cv::Vec3f> colors;
				colors.reserve(grid_num);
				for(size_t r=0; r<rvalues.size(); r++)
					for(size_t g=0; g<gvalues.size(); g++)
						for(size_t b=0; b<bvalues.size(); b++)
							colors.push_back(cv::Vec3f(rvalues[b], gvalues[g], bvalues[r]));

				
				//// draw image
				//int cnt = 0;
				//for(int r=0; r<grid_per_col; r++)
				//{
				//	for(int c=0; c<grid_per_row; c++)
				//	{
				//		cv::Rect cur_box(c*gridsize.width, r*gridsize.height, gridsize.width, gridsize.height);
				//		paletteimg(cur_box).setTo(colors[cnt]);
				//		cnt++;
				//	}
				//}

				//cv::cvtColor(paletteimg, paletteimg, CV_Lab2BGR);
			}
			if(colorspace == COLOR_HSV)
			{
				// generate bins for H, S, V
				vector<float> hvalues;
				vector<float> svalues;
				vector<float> vvalues;
				vector<float> grayvalues;
				// TODO: make sure the interpolation starts from 0 and ends at 1; separate gray value generation
				// Hue
				for(int i=0; i<nums.x; i++)
				{
					float step = 360.f / nums.x;
					hvalues.push_back( step*i );
				}
				// Saturation
				// remove 0 which gives all white
				for(int i=1; i<=nums.y; i++)
				{
					float step = 1.f / nums.y;
					svalues.push_back( step*i );
				}
				// Value
				// remove 0 which gives all black
				for(int i=1; i<=nums.z; i++)
				{
					float step = 1.f / nums.z;
					vvalues.push_back( step*i );
				}
				// separately generate gray values
				int graylevels = svalues.size() + vvalues.size() - 1;
				grayvalues.resize(graylevels);
				for(int i=0; i<graylevels; i++)
				{
					float step = 1.f / graylevels;
					grayvalues[i] = step*i;
				}
				grayvalues[0] = 0;
				grayvalues[graylevels-1] = 1;

				// process color values
				vector<cv::Vec3f> colors;
				// combination of h and s: different colors (normal -> bright)
				for(size_t s=0; s<svalues.size(); s++)
				{
					for(size_t h=0; h<hvalues.size(); h++)
						colors.push_back( cv::Vec3f(hvalues[h], svalues[s], 1) );
				}
				// combination of h and v: different (normal -> dark)
				// remove duplicate colors
				for(int v=vvalues.size()-2; v>=0; v--)
				{
					for(size_t h=0; h<hvalues.size(); h++)
						colors.push_back( cv::Vec3f(hvalues[h], 1, vvalues[v]) );
				}
				
				cout<<colors.size()<<endl;
				// convert to 2d grids
				colorPalette.create(colors.size()/hvalues.size(), hvalues.size(), CV_32FC3);
				for(size_t i=0; i<colors.size(); i++)
					colorPalette.at<Vec3f>(i/hvalues.size(), i%hvalues.size()) = colors[i];

				// process gray values
				grayPalette.create(1, grayvalues.size(), CV_32FC3);
				for(int i=0; i<grayvalues.size(); i++)
					grayPalette.at<Vec3f>(i) = Vec3f(0, 0, grayvalues[i]);

				if(verbose)
				{
					cv::Size gridsize(20,20);
					cv::Mat colorpaletteimg, graypaletteimg;

					// color image
					int total_num = colors.size();
					int grid_per_row = hvalues.size();
					int grid_per_col = total_num / grid_per_row;
					if(grid_per_col*grid_per_row != total_num)
						grid_per_col++;

					colorpaletteimg.create(colorPalette.rows*gridsize.height, colorPalette.cols*gridsize.width, CV_32FC3);
					for(int r=0; r<colorPalette.rows; r++)
						for(int c=0; c<colorPalette.cols; c++)
						{
							cv::Rect curbox(c*gridsize.width, r*gridsize.height, gridsize.width, gridsize.height);
							colorpaletteimg(curbox).setTo(colorPalette.at<Vec3f>(r,c));
						}

					cv::cvtColor(colorpaletteimg, colorpaletteimg, CV_HSV2BGR);
					colorpaletteimg.convertTo(colorpaletteimg, CV_8UC3, 255);
					cv::imshow("cpalette", colorpaletteimg);

					// gray image
					graypaletteimg.create(grayPalette.cols*gridsize.height, gridsize.width, CV_32FC3);
					for(int id=0; id<grayPalette.cols; id++)
					{
						Rect curbox(0, id*gridsize.height, gridsize.width, gridsize.height);
						graypaletteimg(curbox).setTo( grayPalette.at<Vec3f>(grayPalette.cols-id-1) );
					}
					cv::cvtColor(graypaletteimg, graypaletteimg, CV_HSV2BGR);
					graypaletteimg.convertTo(graypaletteimg, CV_8UC3, 255);
					cv::imshow("gpalette", graypaletteimg);

					cv::waitKey(0);
				}

				cvtColor(colorPalette, colorPalette, CV_HSV2BGR);
				cvtColor(grayPalette, grayPalette, CV_HSV2BGR);
			}

			//cout<<colorPalette.at<Vec3f>(0, 10)<<endl;
			// convert to lab for comparison
			cvtColor(colorPalette, colorPalette, CV_BGR2Lab);	// float type: L,A,B will keep original value
			cvtColor(grayPalette, grayPalette, CV_BGR2Lab);

			return true;
		}

		bool ColorDescriptors::MapPixelToPalette(const Vec3f& hsv_val, const Vec3f& lab_val, Point& color_id, int& gray_id, bool& iscolor)
		{
			if(colorPalette.empty() || grayPalette.empty())
			{
				cerr<<"Empty palette"<<endl;
				return false;
			}

			// if gray
			if(hsv_val.val[1] < 0.1)
			{
				iscolor = false;
				gray_id = -1;
				double mindist = SELF_INFINITE;
				for(int id=0; id<grayPalette.cols; id++)
				{
					const Vec3f& temp_val = grayPalette.at<Vec3f>(id);
					double dist = (lab_val.val[0]-temp_val.val[0])*(lab_val.val[0]-temp_val.val[0]) + \
						(lab_val.val[1]-temp_val.val[1])*(lab_val.val[1]-temp_val.val[1]) + \
						(lab_val.val[2]-temp_val.val[2])*(lab_val.val[2]-temp_val.val[2]);
						//norm(lab_val, grayPalette.at<Vec3f>(id));
					if( dist < mindist ) { mindist = dist; gray_id = id; }
				}
			}
			else
			{
				iscolor = true;
				color_id.x = -1;
				color_id.y = -1;
				double mindist = SELF_INFINITE;
				for(int r=0; r<colorPalette.rows; r++)
					for(int c=0; c<colorPalette.cols; c++)
					{
						const Vec3f& temp_val = colorPalette.at<Vec3f>(r, c);
						double dist = (lab_val.val[0]-temp_val.val[0])*(lab_val.val[0]-temp_val.val[0]) + \
							(lab_val.val[1]-temp_val.val[1])*(lab_val.val[1]-temp_val.val[1]) + \
							(lab_val.val[2]-temp_val.val[2])*(lab_val.val[2]-temp_val.val[2]);
						if( dist < mindist ) { mindist = dist; color_id = Point(c, r); }
					}
			}

			return true;
		}

		//////////////////////////////////////////////////////////////////////////

		bool ColorDescriptors::Compute(const cv::Mat& color_img, cv::Mat& feat, const cv::Mat& mask)
		{
			if(params.feat_type == COLOR_FEAT_HIST)
				return ComputeColorHistogram(color_img, feat, mask);
			if(params.feat_type == COLOR_FEAT_CLUSTER)
				return ComputeColorClusters(color_img, feat, mask);

			return true;
		}

		bool ColorDescriptors::ComputeColorHistogram(const cv::Mat& color_img, cv::Mat& feat, const cv::Mat& mask)
		{
			if(color_img.channels() != 3)
			{
				cerr<<"Must input bgr color image."<<endl;
				return false;
			}

			// direct mapping to hist bin
			if( !params.histParams.usePalette )
			{
				// init
				cv::Mat target_img(color_img.rows, color_img.cols, CV_8UC3);
				int ch1_bin_step, ch2_bin_step, ch3_bin_step;
				int ch1_bin_num, ch2_bin_num, ch3_bin_num;

				// init feat
				feat.create(1, params.histParams.hist_bin_nums[3], CV_32F);
				if(params.histParams.color_space == COLOR_LAB)
				{
					cvtColor(color_img, target_img, CV_BGR2Lab);
				}
				if(params.histParams.color_space == COLOR_RGB)
				{
					cvtColor(color_img, target_img, CV_BGR2RGB);
				}
				if(params.histParams.color_space == COLOR_HSV)
				{
					cvtColor(color_img, target_img, CV_BGR2HSV);
				}

				ch1_bin_num = params.histParams.hist_bin_nums[0];
				ch2_bin_num = params.histParams.hist_bin_nums[1];
				ch3_bin_num = params.histParams.hist_bin_nums[2];

				ch1_bin_step = 255 / ch1_bin_num;
				ch2_bin_step = 255 / ch2_bin_num;
				ch3_bin_step = 255 / ch3_bin_num;

				feat.setTo(0);

				// make 3d hist into 1d
				// bin_id = l_id*a_bin_num*b_bin_num + b_id*a_bin_num + a_id
				int sum_pixels = 0;
				for(int r=0; r<target_img.rows; r++)
				{
					for(int c=0; c<target_img.cols; c++)
					{
						if( (!mask.empty() && mask.at<uchar>(r,c) > 0) || mask.empty() )
						{
							cv::Vec3b cur_val = target_img.at<cv::Vec3b>(r,c);
							int ch1_id = MIN(cur_val[0] / ch1_bin_step, ch1_bin_num-1);
							int ch2_id = MIN(cur_val[1] / ch2_bin_step, ch2_bin_num-1);
							int ch3_id = MIN(cur_val[2] / ch3_bin_step, ch3_bin_num-1);
							//int hist_bin_id = ch1_id*ch2_bin_num*ch3_bin_num + ch2_id*ch3_bin_num + ch3_id;

							//feat.at<float>(0, hist_bin_id)++;
							feat.at<float>(0, ch1_id+ch2_bin_num+ch3_bin_num)++;
							feat.at<float>(0, ch2_id+ch3_bin_num)++;
							feat.at<float>(0, ch3_id)++;

							//sum_pixels++;
						}
					}
				}

				// smooth histogram: to enlarge std
				//cv::GaussianBlur(feat, feat, cv::Size(1,3), 0.6f);

				// normalize
				cv::normalize(feat, feat, 1, 0, cv::NORM_L1);
				//feat = feat / sum_pixels;
			}
			else
			{
				if(colorPalette.empty())
				{
					std::cerr<<"ColorDescriptors: Empty palette"<<std::endl;
					return false;
				}

				Mat colorfeat(colorPalette.rows, colorPalette.cols, CV_32F);
				colorfeat.setTo(0);
				Mat grayfeat(1, grayPalette.cols, CV_32F);
				grayfeat.setTo(0);

				cv::Mat cimg_f, labimg, hsvimg;
				color_img.convertTo(cimg_f, CV_32FC3);
				cimg_f /= 255;	// ! very important to get correct conversion for float-type rgb
				cvtColor(cimg_f, labimg, CV_BGR2Lab);
				cvtColor(cimg_f, hsvimg, CV_BGR2HSV);

				//////////////////////////////////////////////////////////////////////////

				for(int r=0; r<labimg.rows; r++)
				{
					for(int c=0; c<labimg.cols; c++)
					{
						if( (!mask.empty() && mask.at<uchar>(r,c) > 0) || mask.empty() )
						{
							cv::Vec3f cur_val = labimg.at<cv::Vec3f>(r, c);
							Point colorid;
							int grayid;
							bool iscolor;
							if( MapPixelToPalette(hsvimg.at<Vec3f>(r, c), labimg.at<Vec3f>(r, c), colorid, grayid, iscolor) )
								 ( iscolor? colorfeat.at<float>(colorid)++: grayfeat.at<float>(grayid)++ );
						}
					}
				}

				/*normalize(colorfeat, colorfeat, 1, 0, NORM_L1);
				imshow("colorfeat", colorfeat*255);
				normalize(grayfeat, grayfeat, 1, 0, NORM_L1);
				imshow("grayfeat", grayfeat*255);
				waitKey(0);*/

				// smooth histogram: to enlarge std
				blur(colorfeat, colorfeat, Size(5, 5));
				blur(grayfeat, grayfeat, Size(1, 5));

				// make final feature
				feat.create(1, colorfeat.rows*colorfeat.cols+grayfeat.cols, CV_32F);
				int cnt = 0;
				for(int r=0; r<colorfeat.rows; r++)
					for(int c=0; c<colorfeat.cols; c++)
						feat.at<float>(cnt++) = colorfeat.at<float>(r, c);
				for(int id=0; id<grayfeat.cols; id++)
					feat.at<float>(cnt++) = grayfeat.at<float>(id);
					
				// normalize
				normalize(feat, feat, 1, 0, NORM_L1);

				//cout<<feat<<endl;
			}

			return true;
		}

		bool ColorDescriptors::ComputeColorClusters(const cv::Mat& color_img, cv::Mat& feat, const cv::Mat& mask)
		{
			if(color_img.channels() != 3)
			{
				cerr<<"Must input bgr color image."<<endl;
				return false;
			}
			
			// assume has been resized
			// convert to lab
			Mat new_img;
			color_img.convertTo(new_img, CV_32FC3);
			new_img /= 255;
			cvtColor(new_img, new_img, CV_BGR2Lab);
	
			// start iterative clustering
			Mat pixelsamps(new_img.rows*new_img.cols, 3, CV_32F);
			if( !mask.empty() )
				pixelsamps.create(countNonZero(mask), 3, CV_32F);
			if(pixelsamps.rows < params.clusterParams.cluster_num)
				return false;

			pixelsamps.setTo(0);
			int curpos = 0;
			for (int r=0; r<new_img.rows; r++) 
				for(int c=0; c<new_img.cols; c++)
				{
					if( mask.empty() || (!mask.empty() && mask.at<uchar>(r,c) > 0) )
					{
						const Vec3f& curpixel = new_img.at<Vec3f>(r, c);
						pixelsamps.at<float>(curpos, 0) = curpixel.val[0];
						pixelsamps.at<float>(curpos, 1) = curpixel.val[1];
						pixelsamps.at<float>(curpos, 2) = curpixel.val[2];
						curpos++;
					}
				}

			// do clustering
			Mat labels, centers;
			kmeans(pixelsamps, params.clusterParams.cluster_num, labels, 
				TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 12, 0.01f), 
				4, KMEANS_PP_CENTERS, centers);

			// set up palette
			colorPalette.create(1, params.clusterParams.cluster_num, CV_32FC3);
			for (size_t i=0; i<centers.rows; i++)
			{
				Vec3f& curmean = colorPalette.at<Vec3f>(i);
				curmean.val[0] = centers.at<float>(i, 0);
				curmean.val[1] = centers.at<float>(i, 1);
				curmean.val[2] = centers.at<float>(i, 2);
			}

			// compute weight
			vector<float> weights(params.clusterParams.cluster_num, 0);
			for(int id=0; id<labels.rows; id++)
			{
				int curid = labels.at<int>(id);
				weights[curid]++;
			}
			// normalize
			for(size_t i=0; i<weights.size(); i++)	weights[i] /= pixelsamps.rows;

			// organize into feat: w1, w2, ..., c1, c2, ...
			feat.create(1, params.clusterParams.cluster_num*4, CV_32F);
			feat.setTo(0);
			// weights
			for (size_t i=0; i<weights.size(); i++) feat.at<float>(i) = weights[i];
			// cluster centers
			curpos = weights.size();
			for (int r=0; r<centers.rows; r++) for(int c=0; c<centers.cols; c++)
				feat.at<float>(curpos++) = centers.at<float>(r, c);

			return true;
		}

		float ColorDescriptors::ColorClusterDist(const cv::Mat& feat1, const cv::Mat& feat2)
		{
			if(feat1.cols != feat2.cols)
				return 1.f;
			//assert(feat1.cols == feat2.cols);

			float dist = 0;

			//double start_t = getTickCount();
			// parse feature
			vector<ColorCluster> clusters1, clusters2;
			int cls_dim = params.clusterParams.cluster_num;
			clusters1.resize(cls_dim), clusters2.resize(cls_dim);
			for(int c=0; c<cls_dim; c++)
			{
				clusters1[c].center_weight = feat1.at<float>(c);
				clusters1[c].center_data.val[0] = feat1.at<float>( cls_dim+c*3 );
				clusters1[c].center_data.val[1] = feat1.at<float>( cls_dim+c*3+1 );
				clusters1[c].center_data.val[2] = feat1.at<float>( cls_dim+c*3+2 );
				clusters2[c].center_weight = feat2.at<float>(c);
				clusters2[c].center_data.val[0] = feat2.at<float>( cls_dim+c*3 );
				clusters2[c].center_data.val[1] = feat2.at<float>( cls_dim+c*3+1 );
				clusters2[c].center_data.val[2] = feat2.at<float>( cls_dim+c*3+2 );
			}

			// compute distance

#define SELF_EMD

#ifdef WEIGHTED_SUM
#pragma region naive weighted distance sum
			// f1->f2
			float dist12 = 0;
			for(size_t i=0; i<fcomp1.size(); i++)
			{
				float curdist = 0;
				for(size_t j=0; j<fcomp2.size(); j++)
				{
					float tdist = (fcomp1[i][0]-fcomp2[j][0])*(fcomp1[i][0]-fcomp2[j][0]) + \
						(fcomp1[i][1]-fcomp2[j][1])*(fcomp1[i][1]-fcomp2[j][1]) + \
						(fcomp1[i][2]-fcomp2[j][2])*(fcomp1[i][2]-fcomp2[j][2]);
					tdist = sqrt(tdist);
					curdist += w2[j]*tdist;
				}
				dist12 += w1[i]*curdist;
			}
			// f2->f1
			float dist21 = 0;
			for(size_t j=0; j<fcomp2.size(); j++)
			{
				float curdist = 0;
				for(size_t i=0; i<fcomp1.size(); i++)
				{
					float tdist = (fcomp1[i][0]-fcomp2[j][0])*(fcomp1[i][0]-fcomp2[j][0]) + \
						(fcomp1[i][1]-fcomp2[j][1])*(fcomp1[i][1]-fcomp2[j][1]) + \
						(fcomp1[i][2]-fcomp2[j][2])*(fcomp1[i][2]-fcomp2[j][2]);
					tdist = sqrt(tdist);
					curdist += w1[i]*tdist;
				}
				dist21 += w2[j]*curdist;
			}

			dist = (dist21 + dist12) / 2;
#pragma endregion
#endif

#ifdef SELF_EMD
#pragma region Approximate EMD distance

			// borrow idea from our ICCV paper, greedy composition
			// compose bins from big to small with most similar ones
			// sort clusters based on area
			sort(clusters1.begin(), clusters1.end(), 
					[](const ColorCluster& a, const ColorCluster& b) { return a.center_weight > b.center_weight; }
			);
			sort(clusters2.begin(), clusters2.end(), 
				[](const ColorCluster& a, const ColorCluster& b) { return a.center_weight > b.center_weight; }
			);

//#define VERBOSE
#ifdef VERBOSE
			// show ranked cluster based on weights
			Mat clsimg(50, clusters1.size()*50, CV_32FC3);
			for(size_t i=0; i<clusters1.size(); i++)
			{
				clsimg(Rect(50*i, 0, 50, 50)).setTo(clusters1[i].center_data);
			}
			cvtColor(clsimg, clsimg, CV_Lab2BGR);
			clsimg.convertTo(clsimg, CV_8UC3, 255);
			imshow("colorblocks", clsimg);
			waitKey(0);
#endif

			// compute pairwise cluster distance
			vector<vector<Point2f>> clsdist12(clusters1.size());
			vector<vector<Point2f>> clsdist21(clusters2.size());
			for(size_t i=0; i<clusters1.size(); i++)
			{
				for(size_t j=0; j<clusters2.size(); j++)
				{
					float curdist = (float)norm(clusters1[i].center_data, clusters2[j].center_data);
					curdist = curdist / params.clusterParams.MAX_DIST;
					curdist = MIN(1, curdist);
					Point2f pt12(j, curdist);
					Point2f pt21(i, curdist);
					clsdist12[i].push_back(pt12);
					clsdist21[j].push_back(pt21);
				}
			}

			// sort
			for(size_t i=0; i<clsdist12.size(); i++)
			{
				sort(clsdist12[i].begin(), clsdist12[i].end(), 
					[](const Point2f& a, const Point2f& b)
				{ return a.y < b.y; }
				);
				sort(clsdist21[i].begin(), clsdist21[i].end(), 
					[](const Point2f& a, const Point2f& b)
				{ return a.y < b.y; }
				);
			}
			
			vector<float> tempw1, tempw2;	// backup weight used for saving temporary values during composition
			tempw1.resize(cls_dim), tempw2.resize(cls_dim);
			// copy weights
			for(size_t i=0; i<clusters1.size(); i++)
			{
				tempw1[i] = clusters1[i].center_weight;
				tempw2[i] = clusters2[i].center_weight;
			}

			float dist12 = 0;	// cost to compose 1 using 2
			for(size_t i=0; i<clsdist12.size(); i++) 
			{
#ifdef VERBOSE
				Mat fillmap(100, 50*cls_dim, CV_32FC3);
				fillmap.setTo(Vec3f(100, 0, 0));
				Rect tarbox(0, 0, fillmap.cols, 50);
				fillmap(tarbox).setTo(clusters1[i].center_data);
				int cnt = 0;

				cout<<i<<": "<<clusters1[i].center_weight<<endl;
#endif

				for(size_t j=0; j<clsdist12[i].size(); j++)
				{
					int bestid = clsdist12[i][j].x;
					if(tempw2[bestid] <= 0)
						continue;

					// compose
					float to_fill_amount = MIN(tempw1[i], tempw2[bestid]);
					dist12 += clsdist12[i][j].y * to_fill_amount;
					// update weight
					tempw1[i] -= to_fill_amount;
					tempw2[bestid] -= to_fill_amount;

#ifdef VERBOSE
					cout<<clsdist12[i][j].y<<endl;
					cout<<clusters1[i].center_data<<" "<<clusters2[bestid].center_data<<endl;
					// add fill color
					Rect curbox(50*cnt, 50, 50, 50);
					fillmap(curbox).setTo(clusters2[bestid].center_data);
					cnt++;
#endif

					if(tempw1[i] <= 0)
						break;
				}

#ifdef VERBOSE
				cvtColor(fillmap, fillmap, CV_Lab2BGR);
				fillmap.convertTo(fillmap, CV_8UC3, 255);
				imshow("fillmap", fillmap);
				waitKey(0);
#endif
			}

			dist = dist12;

#pragma endregion
#endif

#ifdef OPENCV_EMD
#pragma region opencv emd

			// compute ground distance matrix
			Mat costMat(clusters1.size(), clusters2.size(), CV_32F);
			costMat.setTo(0);
			for(size_t i=0; i<clusters1.size(); i++)
			{
				for(size_t j=i; j<clusters2.size(); j++)
				{
					float curdist = (float)norm(clusters1[i].center_data, clusters2[j].center_data);
					curdist = curdist / params.clusterParams.MAX_DIST;
					curdist = MIN(1, curdist);
					costMat.at<float>(i, j) = curdist;
					costMat.at<float>(j, i) = curdist;
				}
			}

			// signature, only for weights
			Mat sig1(clusters1.size(), 1, CV_32F);
			Mat sig2(clusters2.size(), 1, CV_32F);
			for (int i=0; i<clusters1.size(); i++) sig1.at<float>(i) = clusters1[i].center_weight;
			for (int i=0; i<clusters2.size(); i++) sig2.at<float>(i) = clusters2[i].center_weight;

			// compute distance
			double start_t = getTickCount();
			dist = EMD(sig1, sig2, CV_DIST_USER, costMat);
			double timecost = (double)(getTickCount()-start_t) / getTickFrequency();

#pragma endregion
#endif

			//cout<<"Time: "<<(double)(getTickCount()-start_t) / getTickFrequency()<<"s"<<endl;

			return dist;
		}

		//////////////////////////////////////////////////////////////////////////

		bool ColorDescriptors::LearnColorPalette(const vector<cv::Mat>& color_imgs, int maxNum, cv::Mat& palette)
		{
			// cluster each images
			vector<cv::Mat> clusters(color_imgs.size());
			for(size_t i=0; i<color_imgs.size(); i++)
			{

			}

			return true;
		}

		void ColorDescriptors::DrawPalette(const cv::Mat& palette)
		{
			Mat canvas(50, 51*palette.cols, CV_8UC3);
			for(int r=0; r<palette.cols; r++)
			{
				Scalar curcolor;
				curcolor.val[0] = palette.at<Vec3b>(r).val[0];
				curcolor.val[1] = palette.at<Vec3b>(r).val[1];
				curcolor.val[2] = palette.at<Vec3b>(r).val[2];
				Rect curbox(r*50, 0, 50, 50);
				rectangle(canvas, curbox, curcolor, -1);
			}

			imshow("palette", canvas);
			waitKey(0);
		}

		void ColorDescriptors::MapImageToFixedPalette(const cv::Mat& color_img)
		{
			if(colorPalette.empty())
			{
				cerr<<"empty palette"<<endl;
				return;
			}

			Mat newImg(color_img.rows, color_img.cols, CV_32FC3);
			Mat color_img_f, hsvimg, labimg;
			color_img.convertTo(color_img_f, CV_32FC3);
			color_img_f /= 255;
			cvtColor(color_img_f, labimg, CV_BGR2Lab);
			cvtColor(color_img_f, hsvimg, CV_BGR2HSV);

			#pragma omp parallel for
			for(int r=0; r<color_img.rows; r++)
			{
				for(int c=0; c<color_img.cols; c++)
				{
					Point colorid;
					int grayid;
					bool iscolor;
					if( MapPixelToPalette(hsvimg.at<Vec3f>(r, c), labimg.at<Vec3f>(r, c), colorid, grayid, iscolor) )
						newImg.at<Vec3f>(r, c) = ( iscolor? colorPalette.at<Vec3f>(colorid): grayPalette.at<Vec3f>(grayid) );
				}
				//cout<<r<<endl;
			}

			// convert to rgb
			cvtColor(newImg, newImg, CV_Lab2BGR);
			newImg *= 255;
			newImg.convertTo(newImg, CV_8UC3);
			imshow("cmpimg", newImg);
			waitKey(0);
		}

		void ColorDescriptors::MapImageToLearnedPalette(const Mat& color_img)
		{
			if(colorPalette.empty() || colorPalette.rows > 1)
			{
				cerr<<"empty palette or incorrect format"<<endl;
				return;
			}

			Mat newImg(color_img.rows, color_img.cols, CV_32FC3);
			Mat color_img_f, labimg;
			color_img.convertTo(color_img_f, CV_32FC3);
			color_img_f /= 255;
			cvtColor(color_img_f, labimg, CV_BGR2Lab);

			#pragma omp parallel for
			for(int r=0; r<labimg.rows; r++)
			{
				for(int c=0; c<labimg.cols; c++)
				{
					// find the most similar color
					double mindist = SELF_INFINITE;
					int bestid = -1;
					for(int id=0; id<colorPalette.cols; id++)
					{
						double dist = norm(labimg.at<Vec3f>(r,c), colorPalette.at<Vec3f>(id));
						if(dist < mindist) { mindist = dist; bestid = id; }
					}

					newImg.at<Vec3f>(r, c) = colorPalette.at<Vec3f>(bestid);
				}
			}

			// convert to rgb
			cvtColor(newImg, newImg, CV_Lab2BGR);
			newImg *= 255;
			newImg.convertTo(newImg, CV_8UC3);
			imshow("cmpimg", newImg);
			waitKey(0);
		}

		void ColorDescriptors::DrawColorHists(const vector<Mat>& cimgs)
		{
			char str[30];
			vector<Mat> feats(cimgs.size());
			for (size_t i=0; i<cimgs.size(); i++)
			{
				if(cimgs[i].empty())
					continue;

				// compute feature
				Mat newimg;
				Size newsz(300, 300);
				//tools::ToolFactory::compute_downsample_ratio(Size(cimgs[i].cols, cimgs[i].rows), MAX_IMG_DIM, newsz);
				resize(cimgs[i], newimg, newsz);
				Compute(newimg, feats[i]);
				Mat canvas(newsz.height, newsz.width*2, CV_8UC3);
				newimg.copyTo(canvas(Rect(0, 0, newsz.width, newsz.height)));
				Mat histimg;
				tools::ToolFactory::DrawHist(histimg, newsz, newsz.height-5, feats[i]);
				histimg.copyTo(canvas(Rect(newsz.width-1, 0, newsz.width, newsz.height)));
				// show
				sprintf_s(str, "img%d", i);
				imshow(string(str), canvas);
				waitKey(10);
			}

			// compute mutual distance
			Mat feat_dists(feats.size(), feats.size(), CV_32F);
			feat_dists.setTo(0);

			for(size_t i=0; i<feats.size(); i++)
				for(size_t j=i+1; j<feats.size(); j++)
					feat_dists.at<float>(i, j) = feat_dists.at<float>(j, i) = compareHist(feats[i], feats[j], CV_COMP_INTERSECT);
			cout<<feat_dists<<endl;

			waitKey(0);
		}
	}
}


