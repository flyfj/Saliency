#include "ColorDescriptors.h"


namespace visualsearch
{
	namespace features
	{
		ColorDescriptors::ColorDescriptors(void)
		{
			// create a default color palette
			CreateColorPalette(COLOR_HSV, cv::Point3d(10, 4, 4));
		}

		bool ColorDescriptors::CreateColorPalette(FeatColorSpace colorspace, cv::Point3d nums)
		{
			cv::Size gridsize(20,20);
			cv::Mat paletteimg;
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
				for(int r=0; r<rvalues.size(); r++)
					for(int g=0; g<gvalues.size(); g++)
						for(int b=0; b<bvalues.size(); b++)
							colors.push_back(cv::Vec3f(rvalues[b], gvalues[g], bvalues[r]));

				// draw image
				int cnt = 0;
				for(int r=0; r<grid_per_col; r++)
				{
					for(int c=0; c<grid_per_row; c++)
					{
						cv::Rect cur_box(c*gridsize.width, r*gridsize.height, gridsize.width, gridsize.height);
						paletteimg(cur_box).setTo(colors[cnt]);
						cnt++;
					}
				}

				cv::cvtColor(paletteimg, paletteimg, CV_Lab2BGR);
			}
			if(colorspace == COLOR_HSV)
			{
				// generate hues
				vector<float> hvalues;
				vector<float> svalues;
				vector<float> vvalues;
				vector<float> grayvalues;
				for(int i=0; i<nums.x; i++)
				{
					float step = 360.f / nums.x;
					hvalues.push_back( step*i );
				}
				for(int i=0; i<=nums.y; i++)
				{
					float step = 1.f / nums.y;
					svalues.push_back( step*i );
				}
				for(int i=0; i<=nums.z; i++)
				{
					float step = 1.f / nums.z;
					vvalues.push_back( step*i );
				}
				int graylevels = svalues.size()+vvalues.size()-3;
				for(int i=graylevels; i>0; i--)
				{
					float step = 1.f / graylevels;
					grayvalues.push_back( step*i );
				}

				cv::vector<cv::Vec3f> colors;
				int nlevel = 0;
				for(int s=1; s<svalues.size(); s++)
				{
					for(int h=0; h<hvalues.size(); h++)
						colors.push_back( cv::Vec3f(hvalues[h], svalues[s], 1) );
					colors.push_back( cv::Vec3f(0, 0, grayvalues[nlevel]) );
					nlevel++;
				}
				for(int v=vvalues.size()-2; v>0; v--)
				{
					for(int h=0; h<hvalues.size(); h++)
						colors.push_back( cv::Vec3f(hvalues[h], 1, vvalues[v]) );
					colors.push_back( cv::Vec3f(0, 0, grayvalues[nlevel]) );
					nlevel++;
				}
				
				if(verbose)
				{
					int total_num = colors.size();
					int grid_per_row = nums.x + 1;
					int grid_per_col = total_num / grid_per_row;
					if(grid_per_col*grid_per_row != total_num)
						grid_per_col++;
					paletteimg.create(grid_per_col*gridsize.height, grid_per_row*gridsize.width, CV_32FC3);
					// draw image
					int cnt = 0;
					for(int r=0; r<grid_per_col; r++)
					{
						for(int c=0; c<grid_per_row; c++)
						{
							cv::Rect cur_box(c*gridsize.width, r*gridsize.height, gridsize.width, gridsize.height);
							if(cnt < colors.size())
								paletteimg(cur_box).setTo(colors[cnt]);
							cnt++;
						}
					}

					cv::cvtColor(paletteimg, paletteimg, CV_HSV2BGR);

					cv::imshow("palette", paletteimg);
					cv::waitKey(0);
				}

				// convert colors from hsv to lab
				colorPalette.create(1, colors.size(), CV_32FC3);
				for(size_t i=0; i<colors.size(); i++)
					colorPalette.at<cv::Vec3f>(0, i) = colors[i];
				cv::cvtColor(colorPalette, colorPalette, CV_HSV2BGR);
				//colorPalette.convertTo(colorPalette, CV_8U, 255);
			}

			//cout<<colorPalette<<endl;
			cv::cvtColor(colorPalette, colorPalette, CV_BGR2Lab);
			//cout<<colorPalette<<endl;

			return true;
		}

		//////////////////////////////////////////////////////////////////////////

		bool ColorDescriptors::ComputeColorHistogram(const cv::Mat& color_img, cv::Mat& feat, const ColorFeatParams& cparams, const cv::Mat& mask)
		{
			if(color_img.channels() != 3)
			{
				cerr<<"Must input bgr color image."<<endl;
				return false;
			}

			// direct mapping to hist bin
			if( !cparams.usePalette )
			{
				// init
				cv::Mat target_img(color_img.rows, color_img.cols, CV_8UC3);
				int ch1_bin_step, ch2_bin_step, ch3_bin_step;
				int ch1_bin_num, ch2_bin_num, ch3_bin_num;

				// init feat
				if(cparams.color_space == COLOR_LAB)
				{
					feat.create(1, cparams.lab_bin_nums[3], CV_32F);
					cv::cvtColor(color_img, target_img, CV_BGR2Lab);

					ch1_bin_num = cparams.lab_bin_nums[0];
					ch2_bin_num = cparams.lab_bin_nums[1];
					ch3_bin_num = cparams.lab_bin_nums[2];
				}
				if(cparams.color_space == COLOR_RGB)
				{
					feat.create(1, cparams.rgb_bin_nums[3], CV_32F);
					color_img.copyTo(target_img);

					ch1_bin_num = cparams.rgb_bin_nums[0];
					ch2_bin_num = cparams.rgb_bin_nums[1];
					ch3_bin_num = cparams.rgb_bin_nums[2];
				}

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
							int hist_bin_id = ch1_id*ch2_bin_num*ch3_bin_num + ch2_id*ch3_bin_num + ch3_id;

							feat.at<float>(0, hist_bin_id)++;
							//feat.at<float>(0, ch1_id+ch2_bin_num+ch3_bin_num)++;
							//feat.at<float>(0, ch2_id+ch3_bin_num)++;
							//feat.at<float>(0, ch3_id)++;

							sum_pixels++;
						}
					}
				}

				// smooth histogram: to enlarge std
				cv::GaussianBlur(feat, feat, cv::Size(1,3), 0.2f);

				// normalize
				//cv::normalize(feat, feat, 1, 0, cv::NORM_L1);
				feat = feat / sum_pixels;
			}
			else
			{
				if(colorPalette.empty())
				{
					std::cerr<<"ColorDescriptors: Empty palette"<<std::endl;
					return false;
				}

				feat.create(1, colorPalette.cols, CV_32F);
				feat.setTo(0);

				cv::Mat labImg;
				color_img.convertTo(labImg, CV_32FC3);
				labImg /= 255;	// ! very important to get correct conversion for float-type rgb
				//cout<<labImg.row(0)<<endl;
				cv::cvtColor(labImg, labImg, CV_BGR2Lab);

				//cout<<bgrfloat.row(100)<<endl;
				//cout<<labImg.row(100)<<endl;

				int cnt = 0;
				for(int r=0; r<labImg.rows; r++)
				{
					for(int c=0; c<labImg.cols; c++)
					{
						if( (!mask.empty() && mask.at<uchar>(r,c) > 0) || mask.empty() )
						{
							cv::Vec3f cur_val = labImg.at<cv::Vec3f>(r, c);
							//cout<<color_img.at<cv::Vec3b>(r, c)<<"   "<<cur_val<<endl;
							// find most similar color in palette
							double mindist = visualsearch::SELF_INFINITE;
							int bestid = -1;
							for(int id=0; id<colorPalette.cols; id++)
							{
								cv::Vec3f temp_val = colorPalette.at<cv::Vec3f>(0, id);
								double dist = (cur_val.val[0]-temp_val.val[0])*(cur_val.val[0]-temp_val.val[0]) + \
									(cur_val.val[1]-temp_val.val[1])*(cur_val.val[1]-temp_val.val[1]) +\
									(cur_val.val[2]-temp_val.val[2])*(cur_val.val[2]-temp_val.val[2]);
								if(dist < mindist)
								{
									mindist = dist;
									bestid = id;
								}
							}

							feat.at<float>(0, bestid)++;
							//cout<<bestid<<endl;
							cnt++;
						}
					}
				}

				// smooth histogram: to enlarge std
				cv::GaussianBlur(feat, feat, cv::Size(1,3), 0.2f);

				// normalize
				feat = feat / cnt;

				//cout<<feat<<endl;
			}

			return true;
		}

		bool ColorDescriptors::Compute(const cv::Mat& color_img, cv::Mat& feat, 
			const ColorFeatParams& cparams, const cv::Mat& mask)
		{
			if(cparams.feat_type == COLOR_FEAT_HIST)
				return ComputeColorHistogram(color_img, feat, cparams, mask);

			return true;
		}
	}
}


