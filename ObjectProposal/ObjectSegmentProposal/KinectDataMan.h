#pragma once
//////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <NuiApi.h>
#include <iostream>
#include <visualsearch/common/Core2d.h>
#include "Kinect/OpenCVFrameHelper.h"
using namespace Microsoft::KinectBridge;


namespace visualsearch
{
	using namespace visualsearch::common;

	namespace io
	{
		namespace camera
		{
			class KinectDataMan
			{
			private:

				OpenCVFrameHelper m_cvhelper;

			public:
				NUI_IMAGE_RESOLUTION color_reso;
				NUI_IMAGE_RESOLUTION depth_reso;

				KinectDataMan(void);

				bool InitKinect();

				bool GetColorDepth(Mat& cimg, Mat& dmap);

				void ShowColorDepth();
			};
		}
	}
}


