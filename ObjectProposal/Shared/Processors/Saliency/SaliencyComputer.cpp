#include "SaliencyComputer.h"

namespace visualsearch
{
	namespace processors
	{
		namespace attention
		{
			SaliencyComputer::SaliencyComputer(void)
			{
			}
			//////////////////////////////////////////////////////////////////////////

			bool SaliencyComputer::ComputeFT(const Mat& cimg, Mat& salmap)
			{
				try
				{
					CV_Assert(!cimg.empty());

					Mat fimg;
					cimg.convertTo(fimg, CV_32FC3, 1.f/255);
					salmap.create(cimg.size(), CV_32F);
					Mat tImg;
					GaussianBlur(fimg, tImg, Size(3, 3), 0);
					cvtColor(tImg, tImg, CV_BGR2Lab);
					Scalar colorM = mean(tImg);
					for (int r = 0; r < tImg.rows; r++)
					{
						float *s = salmap.ptr<float>(r);
						float *lab = tImg.ptr<float>(r);
						for (int c = 0; c < tImg.cols; c++, lab += 3)
							s[c] = (float)((colorM[0] - lab[0])*(colorM[0] - lab[0]) + 
							(colorM[1] - lab[1])*(colorM[1] - lab[1]) + 
							(colorM[2] - lab[2])*(colorM[2] - lab[2]));
					}
					normalize(salmap, salmap, 0, 1, NORM_MINMAX);

					return true;
				}
				catch(Exception e)
				{
					std::cerr<<e.msg<<endl;
					return false;
				}
			}

			bool SaliencyComputer::ComputeSR(const Mat& cimg, Mat& salmap)
			{
				try
				{
					CV_Assert(!cimg.empty());

					Mat fimg;
					cimg.convertTo(fimg, CV_32FC3, 1.f/255);
					Size sz(64, 64);
					Mat img1f[2], sr1f, cmplxSrc2f, cmplxDst2f;
					cvtColor(fimg, img1f[1], CV_BGR2GRAY);
					resize(img1f[1], img1f[0], sz, 0, 0, CV_INTER_AREA); 

					img1f[1] = Mat::zeros(sz, CV_32F);
					merge(img1f, 2, cmplxSrc2f);
					dft(cmplxSrc2f, cmplxDst2f);
					Mat tmpimgs[2];
					split(cmplxDst2f, tmpimgs);
					magnitude(tmpimgs[0], tmpimgs[1], img1f[0]);
					phase(tmpimgs[0], tmpimgs[1], img1f[1]);

					log(img1f[0], img1f[0]);
					blur(img1f[0], sr1f, Size(3, 3));
					sr1f = img1f[0] - sr1f;

					exp(sr1f, sr1f);
					// get complex number from mag and angle
					for(int r=0; r<sr1f.rows; r++) for(int c=0; c<sr1f.cols; c++)
					{
						cmplxDst2f.at<Vec2f>(r, c) = Vec2f(
							sr1f.at<float>(r,c)*cos(img1f[1].at<float>(r,c)),
							sr1f.at<float>(r,c)*sin(img1f[1].at<float>(r,c)));
					}
					dft(cmplxDst2f, cmplxSrc2f, DFT_INVERSE | DFT_SCALE);
					split(cmplxSrc2f, img1f);

					pow(img1f[0], 2, img1f[0]);
					pow(img1f[1], 2, img1f[1]);
					img1f[0] += img1f[1];

					GaussianBlur(img1f[0], img1f[0], Size(3, 3), 0);
					normalize(img1f[0], img1f[0], 0, 1, NORM_MINMAX);
					resize(img1f[0], salmap, fimg.size(), 0, 0, INTER_CUBIC);
				}
				catch(Exception e)
				{
					return false;
				}
				
			}

			bool SaliencyComputer::ComputeHC(const Mat& cimg, Mat& salmap)
			{
				try
				{
					CV_Assert(!cimg.empty());

					Mat fimg;
					cimg.convertTo(fimg, CV_32FC3, 1.f/255);
					salmap = gcsaliency.GetHC(fimg);

					return true;
				}
				catch (Exception e)
				{
					return false;
				}
			}

			// TODO: some error here
			bool SaliencyComputer::ComputeLC(const Mat& cimg, Mat& salmap)
			{
				try
				{
					CV_Assert(!cimg.empty());

					Mat fimg, img;
					cimg.convertTo(fimg, CV_32FC3, 1.f/255);
					cvtColor(fimg, img, CV_BGR2GRAY);
					img.convertTo(img, CV_8U, 255);
					double f[256], s[256];
					memset(f, 0, 256*sizeof(double));
					memset(s, 0, 256*sizeof(double));
					for (int r = 0; r < img.rows; r++)
					{
						byte* data = img.ptr<byte>(r);
						for (int c = 0; c < img.cols; c++)
							f[data[c]] += 1;
					}
					for (int i = 0; i < 256; i++)
						for (int j = 0; j < 256; j++)
							s[i] += abs(i - j) * f[j];
					Mat sal1f(fimg.size(), CV_64F);
					for (int r = 0; r < img.rows; r++)
					{
						byte* data = img.ptr<byte>(r);
						double* sal = sal1f.ptr<double>(r);
						for (int c = 0; c < img.cols; c++)
							sal[c] = s[data[c]];
					}
					resize(sal1f, salmap, Size(cimg.cols, cimg.rows));
					normalize(salmap, salmap, 0, 1, NORM_MINMAX, CV_32F);

					return true;
				}
				catch(Exception e)
				{
					return false;
				}
			}

			//////////////////////////////////////////////////////////////////////////

			bool SaliencyComputer::ComputeSaliencyMap(const Mat& cimg, SaliencyType saltype, Mat& salmap)
			{
				if(saltype == SAL_FT)
					return ComputeFT(cimg, salmap);
				if(saltype == SAL_SR)
					return ComputeSR(cimg, salmap);
				if(saltype == SAL_HC)
					return ComputeHC(cimg, salmap);

				return true;
			}

		}
	}
}


