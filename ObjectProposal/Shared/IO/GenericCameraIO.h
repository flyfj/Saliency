

#pragma once

#include "Common/common_libs.h"
#include "Common/Tools.h"

using namespace tools;
using namespace cv;
using namespace std;


namespace visualsearch
{
	namespace io
	{
		enum StreamType
		{
			STREAM_COLOR,
			STREAM_DEPTH,
			STREAM_IR
		};


		class GenericCameraIO
		{
		public:

			// create and initialize camera: ready to get frames
			virtual bool InitCamera() = 0;

			// indicate success or not
			virtual bool QueryNextFrame(StreamType type, Mat& frame) = 0;
		};
	}
	
}

