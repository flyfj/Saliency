
#pragma once

#include "../Common/common_libs.h"
#include "../Common/Tools.h"
#include "GenericCameraIO.h"

using namespace tools;
using namespace cv;
using namespace std;


namespace visualsearch
{
	namespace io
	{
		class OpenCVCameraIO: public GenericCameraIO
		{
		private:

			VideoCapture capture;

			bool CheckTypeValidity(StreamType type)
			{
				if(type == STREAM_COLOR)
					return true;
				else
					return false;
			}

		public:

			int camera_id;

			OpenCVCameraIO():camera_id(0) {}

			bool InitCamera();

			bool QueryNextFrame(StreamType type, Mat& frame);
		};
	}
	
}

