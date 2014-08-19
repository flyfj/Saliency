
#include "BingObjectness.h"


namespace visualsearch
{
	namespace processors
	{
		namespace attention
		{
			BingObjectness::BingObjectness(void)
			{
				isBingLoaded = false;
			}

			//////////////////////////////////////////////////////////////////////////

			bool BingObjectness::LoadBing()
			{
				// set dataset for training and testing
				DataSetVOC voc2007("e:\\VOC2007\\");
				//voc2007.loadAnnotations();

				// params
				double base = 2;
				int W = 8;
				int NSS = 2;
				int numPerSz = 130;

				bingObjectness = new Objectness(voc2007, base, W, NSS);
				if( bingObjectness->loadTrainedModel() != 1 )
				{
					cerr<<"Fail to load all bing model."<<endl;
					return false;
				}

				isBingLoaded = true;

				return true;
			}

			bool BingObjectness::GetProposals(const cv::Mat& cimg, vector<ImgWin>& detWins, int winnum, bool showres)
			{
				if( !isBingLoaded && !LoadBing() )
				{
					cerr<<"Bing is not initialized."<<endl;
					return false;
				}

				ValStructVec<float, Vec4i> boxes;
				bingObjectness->getObjBndBoxes(cimg, boxes);

				int validwinnum = MIN(winnum, boxes.size());
				detWins.clear();
				detWins.reserve(validwinnum);
				for (int i=0; i<validwinnum; i++)
				{
					// filter out small windows
					ImgWin curwin = ImgWin( boxes[i].val[0], boxes[i].val[1], boxes[i].val[2]-boxes[i].val[0]-1, boxes[i].val[3]-boxes[i].val[1]-1 );
					if(curwin.area()*1.0f / (cimg.rows*cimg.cols) < 0.2)
						continue;

					curwin.score = 1;
					detWins.push_back(curwin);
				}

				return true;
			}

		}
	}
}



