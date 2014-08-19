
#pragma once

#include "common.h"
#include "Tools.h"
#include "GenericCameraIO.h"

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

