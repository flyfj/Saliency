#include "SaliencyComputer.h"


namespace Saliency
{

	SaliencyComputer::SaliencyComputer(void)
	{
	}


	SaliencyComputer::~SaliencyComputer(void)
	{
	}



	float SaliencyComputer::ComputeSegmentSaliency(const Mat& img, const SegSuperPixelFeature& sp_feat, SaliencyType type)
	{

		if( type == Composition )
		{

		}
		if( type == CenterSurroundHistogramContrast )
		{

			if(lab_img.empty())
				cvtColor(img, lab_img, CV_BGR2Lab);

			// ignore too big segment
			///*if(sp_feat.area >= 0.6*img.rows*img.cols)
			//	return -1;*/

			// define surround context
			// bounding box with 2X width and height
			Rect segment_box = sp_feat.box;
			Point tl_pts(segment_box.tl().x-segment_box.width, segment_box.tl().y-segment_box.height);
			tl_pts.x = MAX(0, tl_pts.x);
			tl_pts.y = MAX(0, tl_pts.y);
			Point br_pts(segment_box.br().x+segment_box.width, segment_box.br().y+segment_box.height);
			br_pts.x = MIN(br_pts.x, img.cols-1);
			br_pts.y = MIN(br_pts.y, img.rows-1);
			Rect context_box(tl_pts, br_pts);
			context_box = Rect(0, 0, img.cols, img.rows);

			// show boxes
			Mat temp_img = img.clone();
			rectangle(temp_img, segment_box, CV_RGB(0,255,0), 1);
			rectangle(temp_img, context_box, CV_RGB(255,255,0), 1);
			imshow("saliency", temp_img);
			waitKey(10);

			// compute context feature
			int count = 0;
			vector<float> context_feat(quantBins[0]+quantBins[1]+quantBins[2], 0);
			for(int y=context_box.tl().y; y<context_box.br().y; y++)
			{
				for(int x=context_box.tl().x; x<context_box.br().x; x++)
				{
					if( SegSuperPixelFeature::InsideSegment(Point(x,y), sp_feat) )
						continue;

					/*Vec3b val = img.at<Vec3b>(y,x);
					float b = val.val[0];
					float g = val.val[1];
					float r = val.val[2];
					int bbin = (int)(b/(255.f/quantBins[0]));
					bbin = ( bbin > quantBins[0]-1? quantBins[0]-1: bbin );
					int gbin = (int)(g/(255.f/quantBins[1]));
					gbin = ( gbin > quantBins[1]-1? quantBins[1]-1: gbin );
					int rbin = (int)(r/(255.f/quantBins[2]));
					rbin = ( rbin > quantBins[2]-1? quantBins[2]-1: rbin );

					context_feat[bbin]++;
					context_feat[quantBins[0]+gbin]++;
					context_feat[quantBins[0]+quantBins[1]+rbin]++;*/

					Vec3b val = lab_img.at<Vec3b>(y,x);
					float l = val.val[0];
					float a = val.val[1];
					float b = val.val[2];
					int lbin = (int)(l/(255.f/quantBins[0]));
					lbin = ( lbin > quantBins[0]-1? quantBins[0]-1: lbin );
					int abin = (int)(a/(255.f/quantBins[1]));
					abin = ( abin > quantBins[1]-1? quantBins[1]-1: abin );
					int bbin = (int)(b/(255.f/quantBins[2]));
					bbin = ( bbin > quantBins[2]-1? quantBins[2]-1: bbin );

					context_feat[lbin]++;
					context_feat[quantBins[0]+abin]++;
					context_feat[quantBins[0]+quantBins[1]+bbin]++;
					count+=3;
				}
			}

			// do normalization
			vector<float> feat1(context_feat.size(), 0);	// segment
			vector<float> feat2(context_feat.size(), 0);	// context
			for(size_t i=0; i<sp_feat.feat.size(); i++)
			{
				feat1[i] = sp_feat.feat[i] / (sp_feat.area*3);
				feat2[i] = context_feat[i] / count;
			}

			// compute distance
			float dist = 0;
			for(size_t i=0; i<feat1.size(); i++)
				dist += (feat1[i]-feat2[i])*(feat1[i]-feat2[i]);
			dist = sqrt(dist);

			if(dist > 1)
				cout<<"error"<<endl;

			return dist;

		}


	}

}

