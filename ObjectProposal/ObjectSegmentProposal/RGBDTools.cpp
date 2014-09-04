//////////////////////////////////////////////////////////////////////////
// implementation
// jiefeng@2014-1-4
//////////////////////////////////////////////////////////////////////////


#include "RGBDTools.h"

namespace visualsearch
{
	namespace common
	{
		namespace tools
		{
			bool RGBDTools::LoadMat(const std::string& filename, cv::Mat& rmat, int w, int h)
			{
				std::ifstream in(filename);
				if( !in.is_open() )
					return false;

				rmat.create(h, w, CV_32F);
				for(int r=0; r<h; r++)
				{
					for(int c=0; c<w; c++)
					{
						in>>rmat.at<float>(r,c);
					}
				}

				return true;
			}

			bool RGBDTools::LoadBinaryDepthmap(const std::string& filename, cv::Mat& dmap, int w, int h)
			{
				dmap.create(h, w, CV_32F);

				std::ifstream in(filename, std::ios::binary);
				if( !in.is_open() )
					return false;

				// get file size
				in.seekg (0, in.end);
				int length = in.tellg();
				in.seekg (0, in.beg);

				// verify
				assert( length == w*h*sizeof(float) );

				// read data
				std::vector<float> data(length / sizeof(float) + 1);
				in.read((char*)(&data[0]), length);

				for(int r=0; r<h; r++)
				{
					for(int c=0; c<w; c++)
						dmap.at<float>(r,c) = data[r*w+c];
				}

				return true;
			}

			bool RGBDTools::OutputMaskToFile(std::ofstream& out, const cv::Mat& color_img, const cv::Mat& mask, bool hasProb /* = false */)
			{
				if(mask.empty())
				{
					std::cerr<<"Empty mask."<<std::endl;
					return false;
				}

				for(int r=0; r<mask.rows; r++)
				{
					for(int c=0; c<mask.cols; c++)
					{
						if(hasProb)
						{
							// TODO: implement this
							cv::Vec3b cur_color = color_img.at<cv::Vec3b>(r,c);
							//out<<(c==0? "": " ")<<(mask.at<uchar>(r,c)>0? )
						}
						else
							out<<(c==0? "": " ")<<(int)mask.at<uchar>(r,c);
					}
					out<<std::endl;
				}

				return true;
			}

			bool RGBDTools::SavePointsToOBJ(const string& filename, const Mat& pts)
			{
				std::ofstream out(filename.c_str());
				for (int r=0; r<pts.rows; r++)
				{
					for(int c=0; c<pts.cols; c++)
					{
						Vec3f curval = pts.at<Vec3f>(r, c);
						out<<"v "<<curval.val[0]<<" "<<curval.val[1]<<" "<<curval.val[2]<<std::endl;
					}
				}

				return true;
			}

			//////////////////////////////////////////////////////////////////////////

			

			//////////////////////////////////////////////////////////////////////////

			bool RGBDTools::Proj2Dto3D(const cv::Mat& fg_mask, const cv::Mat& dmap, const cv::Mat& w2c_mat, std::vector<cv::Vec3f>& pts3d)
			{
				// homogeneous coordinates: (x, y, d)
				cv::Mat homo_coords(0, 0, CV_32F);
				cv::Mat dvalmap(0, 0, CV_32F);
				for(int r=0; r<fg_mask.rows; r++)
				{
					for(int c=0; c<fg_mask.cols; c++)
					{
						if(fg_mask.at<uchar>(r,c) > 0)
						{
							float dval = dmap.at<float>(r,c);
							cv::Vec3f cur_pt(c, r, dval);
							homo_coords.push_back( cv::Mat(cur_pt).t() );
							cv::Vec3f cur_dval(dval, dval, dval);
							dvalmap.push_back( cv::Mat(cur_dval).t() );
						}
					}
				}

				// convert to local coordinates (x, y, z)
				homo_coords = homo_coords / dvalmap;

				return true;
			}

			bool RGBDTools::KinectDepthTo3D(const Mat& dmap, Mat& pts3d)
			{
				// dmap is in millimeter
				Point2f centerp(dmap.cols/2, dmap.rows/2);
				int imgh = dmap.rows, imgw = dmap.cols;
				float constant = 585.6f;
				float MM_PER_M = 1000;

				// transform
				pts3d.create(dmap.rows, dmap.cols, CV_32FC3);
				pts3d.setTo(Vec3f(0, 0, 0));
				for (int r=0; r<dmap.rows; r++) {
					for(int c=0; c<dmap.cols; c++) {
						float dval = dmap.at<float>(r, c);
						if( dval == 0 )
							continue;

						Vec3f& curval = pts3d.at<Vec3f>(r, c);
						curval.val[0] = (c-centerp.x) * dval / constant / MM_PER_M;
						curval.val[1] = (r-centerp.y) * dval / constant / MM_PER_M;
						curval.val[2] = dval / MM_PER_M;
					}
				}

				return true;
			}
		}
	}
	
}

