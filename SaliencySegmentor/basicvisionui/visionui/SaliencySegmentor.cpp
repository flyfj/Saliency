//////////////////////////////////////////////////////////////////////////


#include "SaliencySegmentor.h"

namespace Saliency
{
	SaliencySegmentor::SaliencySegmentor(void)
	{
	}


	SaliencySegmentor::~SaliencySegmentor(void)
	{
	}

	void SaliencySegmentor::Init(const Mat& img)
	{

		// do segmentation
		img_segmentor.m_dMinArea = 100;
		img_segmentor.m_dSmoothSigma = 0.5f;
		img_segmentor.m_dThresholdK = 100.f;
		int segment_num = img_segmentor.DoSegmentation(img);
		cout<<"Total segments number: "<<segment_num<<endl;
		max_id = segment_num-1;

		// create first level segments
		//////////////////////////////////////////////////////////////////////////
		// compute features for each superpixel
		//////////////////////////////////////////////////////////////////////////
		// create data
		sp_features.clear();
		sp_features.resize(segment_num);
		merged_sign.clear();
		merged_sign.resize(segment_num, false);

		// compute perimeter and boundary pixel numbers, set bounding box
		for(int y=0; y<img.rows; y++)
		{
			for(int x=0; x<img.cols; x++)
			{
				int seg_id = img_segmentor.m_idxImg.at<int>(y,x);
				
				// add area
				sp_features[seg_id].area++;
				// add centroid
				sp_features[seg_id].centroid.x += x;
				sp_features[seg_id].centroid.y += y;

				// update bounding box points
				sp_features[seg_id].box_pos[0].x = min(sp_features[seg_id].box_pos[0].x, x);
				sp_features[seg_id].box_pos[0].y = min(sp_features[seg_id].box_pos[0].y, y);
				sp_features[seg_id].box_pos[1].x = max(sp_features[seg_id].box_pos[1].x, x);
				sp_features[seg_id].box_pos[1].y = max(sp_features[seg_id].box_pos[1].y, y);

				// sum up boundary pixel number
				if(x==0 || x==img.cols-1 || y==0 || y==img.rows-1)
				{
					sp_features[seg_id].bnd_pixels++;
					sp_features[seg_id].perimeter++;	// boundary pixel must be in perimeter
					continue;
				}

				// perimeter and neighbor relation
				int left_idx = img_segmentor.m_idxImg.at<int>(y, x-1);
				if(seg_id != left_idx)
				{
					sp_features[seg_id].perimeter++;
					sp_features[seg_id].neighbor_ids.insert(left_idx);
					sp_features[left_idx].neighbor_ids.insert(seg_id);
					continue;
				}
				int right_idx = img_segmentor.m_idxImg.at<int>(y,x+1);
				if(seg_id != right_idx)
				{ 
					sp_features[seg_id].perimeter++;
					sp_features[seg_id].neighbor_ids.insert(right_idx);
					sp_features[right_idx].neighbor_ids.insert(seg_id);
					continue;
				}
				int top_idx = img_segmentor.m_idxImg.at<int>(y-1,x);
				if(seg_id != top_idx)
				{ 
					sp_features[seg_id].perimeter++;
					sp_features[seg_id].neighbor_ids.insert(top_idx);
					sp_features[top_idx].neighbor_ids.insert(seg_id); 
					continue;
				}
				int bottom_idx = img_segmentor.m_idxImg.at<int>(y+1, x);
				if(seg_id != bottom_idx)
				{ 
					sp_features[seg_id].perimeter++;
					sp_features[seg_id].neighbor_ids.insert(bottom_idx);
					sp_features[bottom_idx].neighbor_ids.insert(seg_id);
					continue; 
				}

				if(use4Neighbor) continue;
				// 8 neighbor case
				/*if(seg_id != img_segmentor.m_idxImg.at<int>(y-1, x-1))
				{ sp_features[seg_id].perimeter++; continue; }
				if(seg_id != img_segmentor.m_idxImg.at<int>(y-1, x+1))
				{ sp_features[seg_id].perimeter++; continue; }
				if(seg_id != seg_index_map.Pixel(x-1, y+1))
				{ sp_features[seg_id].perimeter++; continue; }
				if(seg_id != seg_index_map.Pixel(x+1, y+1))
				{ sp_features[seg_id].perimeter++; continue; }*/

			}
		}

		// compute bounding box and centroid of each segment
		for(size_t i=0; i<sp_features.size(); i++)
		{
			sp_features[i].id = i;
			sp_features[i].feat.resize(quantBins[0]+quantBins[1]+quantBins[2], 0);
			//	set bound box
			sp_features[i].box = \
				Rect(sp_features[i].box_pos[0].x, sp_features[i].box_pos[0].y, \
				sp_features[i].box_pos[1].x-sp_features[i].box_pos[0].x+1, \
				sp_features[i].box_pos[1].y-sp_features[i].box_pos[0].y+1);
			// init mask
			sp_features[i].mask.create(img.rows, img.cols, CV_8U);
			sp_features[i].mask.setTo(0);
			// centroid
			sp_features[i].centroid.x /= sp_features[i].area;
			sp_features[i].centroid.y /= sp_features[i].area;
			// init components to itself only
			sp_features[i].components.push_back(i);
		}

		// set mask
		for(int y=0; y<img.rows; y++)
		{
			for(int x=0; x<img.cols; x++)
			{
				int seg_id = img_segmentor.m_idxImg.at<int>(y,x);
				sp_features[seg_id].mask.at<uchar>(y,x) = 1;
			}
		}


		// compute appearance feature: LAB histogram
		cvtColor(img, lab_img, CV_BGR2Lab);	// output 0~255
		for(int y=0; y<img.rows; y++)
		{
			for(int x=0; x<img.cols; x++)
			{
				
				int seg_id = img_segmentor.m_idxImg.at<int>(y,x);

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

				sp_features[seg_id].feat[bbin]++;
				sp_features[seg_id].feat[quantBins[0]+gbin]++;
				sp_features[seg_id].feat[quantBins[0]+quantBins[1]+rbin]++;*/

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

				sp_features[seg_id].feat[lbin]++;
				sp_features[seg_id].feat[quantBins[0]+abin]++;
				sp_features[seg_id].feat[quantBins[0]+quantBins[1]+bbin]++;
			}
		}

		// no normalization here, allow easy computation of merged segment feature

		//do feature normalization
		/*for(size_t i=0; i<sp_features.size(); i++)
		{
			SuperPixel& curfeat = sp_features[i];
			for(size_t j=0; j<curfeat.feat.size(); j++)
				curfeat.feat[j] /= (3*curfeat.area);
		}*/

	}


	float SaliencySegmentor::MergeSegments(
		const SegSuperPixelFeature& in_seg1, const SegSuperPixelFeature& in_seg2, 
		SegSuperPixelFeature& out_seg, bool onlyCombineFeat)
	{

		// combine masks
		out_seg.mask = in_seg1.mask | in_seg2.mask;
		out_seg.area = countNonZero(out_seg.mask);
		Mat backup_mask = out_seg.mask.clone();
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours( backup_mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0) );
		
		if(contours.empty())
		{
			cerr<<"error combining masks."<<endl;
			return -1;
		}

		out_seg.box = boundingRect(contours[0]);
		out_seg.box_pos[0] = out_seg.box.tl();
		out_seg.box_pos[1] = out_seg.box.br();
		out_seg.perimeter = contours[0].size();

		// feature
		out_seg.feat.resize(in_seg1.feat.size());
		for(size_t i=0; i<in_seg1.feat.size(); i++)
			out_seg.feat[i] = in_seg1.feat[i] + in_seg2.feat[i];

		// after merge segment id always -1 to distinguish from others
		out_seg.id = -1;

		// compute segment color histogram similarity
		float dist = 0;
		vector<float> feat1 = in_seg1.feat;
		vector<float> feat2 = in_seg2.feat;
		// normalize
		for(size_t i=0; i<feat1.size(); i++)
		{
			feat1[i] /= in_seg1.area*3;
			feat2[i] /= in_seg2.area*3;
		}

		for(size_t i=0; i<feat1.size(); i++)
			dist += (feat1[i]-feat2[i])*(feat1[i]-feat2[i]);
		dist = sqrt(dist);


		// merge components
		out_seg.components = in_seg1.components;
		for(size_t i=0; i<in_seg2.components.size(); i++)
			out_seg.components.push_back(in_seg2.components[i]);


		if(onlyCombineFeat)
			return dist;

		// update adjacency matrix
		if(in_seg1.id != -1)
			merged_sign[in_seg1.id] = true;
		if(in_seg2.id != -1)
			merged_sign[in_seg2.id] = true;

		// add neighbors of seg2 to seg1 list except merged ones
		out_seg.neighbor_ids.clear();
		for( set<int>::iterator pi=in_seg1.neighbor_ids.begin(); pi!=in_seg1.neighbor_ids.end(); pi++ )
		{
			if( !merged_sign[*pi] )
				out_seg.neighbor_ids.insert(*pi);
		}
		for( set<int>::iterator pi=in_seg2.neighbor_ids.begin(); pi!=in_seg2.neighbor_ids.end(); pi++ )
		{
			if( !merged_sign[*pi] )
				out_seg.neighbor_ids.insert(*pi);
		}

		return dist;

	}


	float SaliencySegmentor::SegmentDissimilarity(const SegSuperPixelFeature& seg1, const SegSuperPixelFeature& seg2)
	{

		// select primitive segment
		int mergedSegId, singleSegId;
		if(seg1.components.size() == 1 && seg2.components.size()==1)
		{
			if(merged_sign[seg1.id])
				mergedSegId = 1;
			else
				mergedSegId = 2;
		}
		else
		{
			mergedSegId = (seg1.components.size()>1? 1: 2);
		}

		singleSegId = (mergedSegId==1? 2: 1);

		const SegSuperPixelFeature& mergedSeg = (mergedSegId==1? seg1: seg2);
		const SegSuperPixelFeature& singleSeg = (singleSegId==1? seg1: seg2);

		float dist = 0;
		vector<float> feat1 = mergedSeg.feat;
		vector<float> feat2 = singleSeg.feat;
		// normalize
		for(size_t i=0; i<feat1.size(); i++)
		{
			feat1[i] /= seg1.area*3;
			feat2[i] /= seg2.area*3;
		}

		for(size_t i=0; i<feat1.size(); i++)
			dist += (feat1[i]-feat2[i])*(feat1[i]-feat2[i]);
		dist = sqrt(dist);

		return dist;


		float min_dist = INFINITE;
		for(set<int>::iterator pi=singleSeg.neighbor_ids.begin(); pi!=singleSeg.neighbor_ids.end(); pi++)
		{
			if(merged_sign[*pi])
			{
				// compute distance of two segments as the distance between two most similar segment components
				float dist = 0;
				vector<float> feat1 = sp_features[*pi].feat;
				vector<float> feat2 = singleSeg.feat;
				// normalize
				for(size_t i=0; i<feat1.size(); i++)
				{
					feat1[i] /= seg1.area*3;
					feat2[i] /= seg2.area*3;
				}

				for(size_t i=0; i<feat1.size(); i++)
					dist += (feat1[i]-feat2[i])*(feat1[i]-feat2[i]);
				dist = sqrt(dist);

				if(dist < min_dist)
					min_dist = dist;
			}
		}
		

		return min_dist;
	}


	bool SaliencySegmentor::MineSalientObjectFromSegment(const Mat& img, int start_seg_id, float& best_saliency)
	{
		if(sp_features.empty())
		{
			cerr<<"SaliencySegmentor: not init yet."<<endl;
			return false;
		}

		// reset merged sign
		for(size_t i=0; i<merged_sign.size(); i++)
			merged_sign[i] = false;

		merged_sign[start_seg_id] = true;

		// globally best salient object
		best_saliency = 0;
		Mat best_obj_img;

		SegSuperPixelFeature cur_merge = sp_features[start_seg_id];
		while( !cur_merge.neighbor_ids.empty() )
		{
			// find most salient neighbor to merge
			float max_merge = 0;
			float max_saliency = 0;
			int best_id = -1;
			// check each neighbor
			Mat neighbor_mask(img.rows, img.cols, CV_8U);
			neighbor_mask.setTo(0);
			for(set<int>::iterator pi=cur_merge.neighbor_ids.begin(); pi!=cur_merge.neighbor_ids.end(); pi++)
			{
				SegSuperPixelFeature merged_seg;
				neighbor_mask.setTo(255, sp_features[*pi].mask);

				// compute distance with each neighbor
				float dist = SegmentDissimilarity(cur_merge, sp_features[*pi]);
				cout<<dist<<endl;
				// do dummy merge (only need updated mask and combined feature)
				MergeSegments(cur_merge, sp_features[*pi], merged_seg, true);
				// compute saliency score after merge
				float sal_score = sal_computer.ComputeSegmentSaliency(img, merged_seg, CenterSurroundHistogramContrast);
				float merge_score =	1 - dist;	// more similar (dist smaller) and more salient after merge is preferred
				if(merge_score > max_merge)
				{
					max_merge = merge_score;
					max_saliency = sal_score;
					best_id = *pi;
				}
				/*if(sal_score > max_saliency)
					max_saliency = sal_score;*/
			}

			//cout<<"Max merge score: "<<max_merge<<endl;
			cout<<"Max saliency: "<<max_saliency<<endl;

			// do actual merge with the best one
			SegSuperPixelFeature temp_merged;
			MergeSegments(cur_merge, sp_features[best_id], temp_merged, false);
			// update
			cur_merge = temp_merged;

			// show merged segment
			Mat cur_obj_img(img.rows, img.cols, img.depth());
			cur_obj_img.setTo(255);
			img.copyTo(cur_obj_img, cur_merge.mask);
			imshow("cur_obj", cur_obj_img);
			imshow("neighbors", neighbor_mask);
			Mat graymask;
			sp_features[best_id].mask.convertTo(graymask, CV_8U, 255);
			imshow("best neighbor", graymask);
			waitKey(0);

			if(max_saliency > best_saliency && cur_merge.area < img.rows*img.cols*0.6)
			{
				best_saliency = max_saliency;
				cur_obj_img.copyTo(best_obj_img);
			}

		}

		cout<<"Best saliency score: "<<best_saliency<<" id: "<<start_seg_id<<endl;
		imshow("Best salient object", best_obj_img);
		waitKey(0);


		return true;

	}


	bool SaliencySegmentor::ComputeSaliencyMap(const Mat& img, Mat& sal_map)
	{
		sal_map.create(img.rows, img.cols, CV_32F);
		sal_map.setTo(0);

		for(size_t i=0; i<sp_features.size(); i++)
		{
			float score = 0;
			MineSalientObjectFromSegment(img, i, score);
			sal_map.setTo(score, sp_features[i].mask);
		}

		return true;
	}

}

