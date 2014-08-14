#include "EdgeHistogram.h"


namespace visualsearch
{
	namespace features
	{
		EdgeHistogram::EdgeHistogram(void)
		{
		}

		void EdgeHistogram::Init(const EdgeFeatParams& params)
		{
			eparams = params;
		}

		bool EdgeHistogram::Compute(const cv::Mat& gray_img, cv::Mat& edgehist, const cv::Mat& mask)
		{
			if(gray_img.channels() != 1)
			{
				std::cerr<<"Input of edge hist must be gray image."<<std::endl;
				return false;
			}

			// compute gradient
			cv::Mat grad_x, grad_y, grad_mag;
			cv::Sobel( gray_img, grad_x, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT );
			cv::Sobel( gray_img, grad_y, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT );
			cv::magnitude(grad_x, grad_y, grad_mag);

			//cv::imshow("grad", grad_mag);
			//cv::waitKey(10);

			// angle
			cv::Mat grad_ang;		// 0~360: change to 0~180
			cv::phase(grad_x, grad_y, grad_ang, true);
			
			// init
			vector<Mat> histfeats(eparams.multi_bin_num.size());
			vector<float> bin_steps(eparams.multi_bin_num.size());
			int binsum = 0;
			for(size_t i=0; i<eparams.multi_bin_num.size(); i++) 
			{
				binsum += eparams.multi_bin_num[i];
				bin_steps[i] = 180 / eparams.multi_bin_num[i];
				histfeats[i].create(1, eparams.multi_bin_num[i], CV_32F);
				histfeats[i].setTo(0);
			}
			
			// angle accumulation
			for(int r=0; r<grad_mag.rows; r++)
			{
				for(int c=0; c<grad_mag.cols; c++)
				{
					if( mask.empty() || (!mask.empty() && mask.at<uchar>(r,c)>0) )
					{
						float mag = grad_mag.at<float>(r, c);
						float angle = grad_ang.at<float>(r, c);
						// trunc into 0~180
						angle = (angle >= 180? angle-180: angle);
						// binning
						for (int i=0; i<eparams.multi_bin_num.size(); i++)
						{
							int bin_id = MIN(eparams.multi_bin_num[i]-1, (int)angle/bin_steps[i]);
							histfeats[i].at<float>(0, bin_id) += mag;
						}
					}
				}
			}
			
			// concatenate to one feature
			edgehist.create(1, binsum, CV_32F);
			edgehist.setTo(0);
			int cnt = 0;
			for (size_t i=0; i<histfeats.size(); i++)
			{
				for (int c=0; c<histfeats[i].cols; c++)
					edgehist.at<float>(0, cnt++) = histfeats[i].at<float>(c);
			}

			// normalize hists
			normalize(edgehist, edgehist, 1, 0, NORM_L1);

			return true;
		}
	}
}



