//////////////////////////////////////////////////////////////////////////
// bing objectness cvpr2014
// jiefeng@2014-08-17
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "../../Common/common_libs.h"
#include "Bing/Objectness.h"

namespace visualsearch
{
	namespace processors
	{
		namespace attention
		{
			class BingObjectness
			{
			private:
				bool isBingLoaded;

				Objectness* bingObjectness;

			public:
				BingObjectness(void);

				bool LoadBing();

				bool TrainBing();

				bool GetProposals(const cv::Mat& cimg, vector<ImgWin>& detWins, int winnum, bool showres=false);

			};
		}
	}
}


