#include "KinectDataMan.h"


namespace visualsearch
{
	namespace io
	{
		namespace camera
		{
			KinectDataMan::KinectDataMan(void)
			{
				color_reso = NUI_IMAGE_RESOLUTION_640x480;
				depth_reso = NUI_IMAGE_RESOLUTION_640x480;
			}

			//////////////////////////////////////////////////////////////////////////

			bool KinectDataMan::InitKinect()
			{
				if( m_cvhelper.IsInitialized() )
					return true;

				// set depth to near mode
				m_cvhelper.SetDepthStreamFlag(NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE, true);
				m_cvhelper.SetColorFrameResolution(color_reso);
				m_cvhelper.SetDepthFrameResolution(depth_reso);

				HRESULT hr;
				// Get number of Kinect sensors
				int sensorCount = 0;
				hr = NuiGetSensorCount(&sensorCount);
				if (FAILED(hr)) 
				{
					return false;
				}

				// If no sensors, update status bar to report failure and return
				if (sensorCount == 0)
				{
					cerr<<"No kinect"<<endl;
					return false;
				}

				// Iterate through Kinect sensors until one is successfully initialized
				for (int i = 0; i < sensorCount; ++i) 
				{
					INuiSensor* sensor = NULL;
					hr = NuiCreateSensorByIndex(i, &sensor);
					if (SUCCEEDED(hr))
					{
						hr = m_cvhelper.Initialize(sensor);
						if (SUCCEEDED(hr)) 
						{
							// Report success
							cerr<<"Kinect initialized"<<endl;
							break;
						}
						else
						{
							// Uninitialize KinectHelper to show that Kinect is not ready
							m_cvhelper.UnInitialize();
							cerr<<"Fail to init kinect."<<endl;

							return false;
						}
					}
				}

				return true;
			}

			bool KinectDataMan::GetColorDepth(Mat& cimg, Mat& dmap)
			{
				DWORD width, height;
				m_cvhelper.GetColorFrameSize(&width, &height);
				Size colorsz(width, height);
				cimg.create(colorsz, m_cvhelper.COLOR_TYPE);
				m_cvhelper.GetDepthFrameSize(&width, &height);
				Size depthsz(width, height);
				dmap.create(depthsz, m_cvhelper.DEPTH_TYPE);

				// get color frame
				if( SUCCEEDED(m_cvhelper.UpdateColorFrame()) )
				{
					HRESULT hr = m_cvhelper.GetColorImage(&cimg);
					if(FAILED(hr))
					{
						cerr<<"Fail to get color image"<<endl;
						return false;
					}

					cvtColor(cimg, cimg, CV_BGRA2BGR);
				}
				else
				{
					cerr<<"Can't get color frame"<<endl;
					return false;
				}

				if( SUCCEEDED(m_cvhelper.UpdateDepthFrame()) )
				{
					HRESULT hr = m_cvhelper.GetDepthImage(&dmap);
					if(FAILED(hr))
					{
						cerr<<"Fail to get depth image"<<endl;
						return false;
					}

					if (cimg.channels() == 3) {
						// refine depth map
						// map color to depth will cause empty color pixels
						// here we use a reverse way to fill depth
						Mat new_dmap = Mat::zeros(depthsz, CV_16U);
						Mat new_cimg = Mat::zeros(colorsz, CV_8UC3);
						for (int r = 0; r < dmap.rows; r++) for (int c = 0; c < dmap.cols; c++) {
							USHORT dval = dmap.at<USHORT>(r, c);
							if (dval == 0) continue;
							LONG c_x, c_y;
							NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
								color_reso,												// color frame resolution
								depth_reso,												// depth frame resolution
								NULL,													// pan/zoom of color image (IGNORE THIS)
								c, r, dval,					// Column, row, and depth in depth image
								&c_x, &c_y												// Output: column and row (x,y) in the color image
								);
							if (c_x >= cimg.cols || c_x < 0 || c_y >= cimg.rows || c_y < 0)
								continue;
							new_dmap.at<USHORT>(c_y, c_x) = dval;
							//new_cimg.at<Vec3b>(r, c) = cimg.at<Vec3b>(c_y, c_x);
						}

						new_dmap.copyTo(dmap);
						//new_cimg.copyTo(cimg);
					}
	
				}
				else
				{
					cerr<<"Can't get depth frame"<<endl;
					return false;
				}

				return true;
			}

			void KinectDataMan::ShowColorDepth()
			{
				// init kinect and connect
				if( m_cvhelper.IsInitialized() )
					return;

				m_cvhelper.SetColorFrameResolution(color_reso);
				m_cvhelper.SetDepthFrameResolution(depth_reso);

				HRESULT hr;
				// Get number of Kinect sensors
				int sensorCount = 0;
				hr = NuiGetSensorCount(&sensorCount);
				if (FAILED(hr)) 
				{
					return;
				}

				// If no sensors, update status bar to report failure and return
				if (sensorCount == 0)
				{
					cerr<<"No kinect"<<endl;
				}

				// Iterate through Kinect sensors until one is successfully initialized
				for (int i = 0; i < sensorCount; ++i) 
				{
					INuiSensor* sensor = NULL;
					hr = NuiCreateSensorByIndex(i, &sensor);
					if (SUCCEEDED(hr))
					{
						hr = m_cvhelper.Initialize(sensor);
						if (SUCCEEDED(hr)) 
						{
							// Report success
							cerr<<"Kinect initialized"<<endl;
							break;
						}
						else
						{
							// Uninitialize KinectHelper to show that Kinect is not ready
							m_cvhelper.UnInitialize();
							return;
						}
					}
				}

				DWORD width, height;
				m_cvhelper.GetColorFrameSize(&width, &height);
				Size colorsz(width, height);
				Mat cimg(colorsz, m_cvhelper.COLOR_TYPE);
				m_cvhelper.GetDepthFrameSize(&width, &height);
				Size depthsz(width, height);
				Mat dimg(depthsz, m_cvhelper.DEPTH_RGB_TYPE);

				// start processing
				while(true)
				{
					// get color frame
					if( SUCCEEDED(m_cvhelper.UpdateColorFrame()) )
					{
						HRESULT hr = m_cvhelper.GetColorImage(&cimg);
						if(FAILED(hr))
							break;

						imshow("color", cimg);
						if( waitKey(10) == 'q' )
							break;
					}

					if( SUCCEEDED(m_cvhelper.UpdateDepthFrame()) )
					{
						HRESULT hr = m_cvhelper.GetDepthImageAsArgb(&dimg);
						if(FAILED(hr))
							break;

						imshow("depth", dimg);
						if( waitKey(10) == 'q' )
							break;
					}
				}

				destroyAllWindows();

			}
		}
	}
}

