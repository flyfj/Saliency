
#include "OpenCVCameraIO.h"


namespace visualsearch
{
	namespace io
	{
		bool OpenCVCameraIO::InitCamera()
		{
			// find first connected camera
			capture.open(camera_id);
			if(!capture.isOpened())
			{
				std::cerr<<"Fail to init camera: "<<camera_id<<endl;
				return false;
			}

			return true;
		}

		bool OpenCVCameraIO::QueryNextFrame(StreamType type, Mat& frame)
		{
			if( !capture.isOpened() )
			{
				cerr<<"Camera is not initialized."<<endl;
				return false;
			}

			if( !CheckTypeValidity(type) )
			{
				cerr<<"query stream type not supported by opencv camera."<<endl;
				return false;
			}

			capture >> frame;
			if( frame.empty() )
				return false;

			return true;
		}
	}
	

}