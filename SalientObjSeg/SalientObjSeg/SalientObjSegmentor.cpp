#include "SalientObjSegmentor.h"



// initialize static member
int SalientObjSegmentor::width = 0;
int SalientObjSegmentor::height = 0;
float SalientObjSegmentor::tradeoff = 0;
float SalientObjSegmentor::bias = 0;
Mat SalientObjSegmentor::cur_spFGWeightmap = Mat();
Mat SalientObjSegmentor::seedMask = Mat();
Mat SalientObjSegmentor::bgMask = Mat();
Mat SalientObjSegmentor::norm_img = Mat();
//////////////////////////////////////////////////////////////////////////
// unary cost
MRF::CostVal dCost(int pix, int i)
{
	MRF::CostVal val = 0;
	Point pix_pos;
	pix_pos.x = pix % SalientObjSegmentor::width;
	pix_pos.y = pix / SalientObjSegmentor::width;
	
	float sval = SalientObjSegmentor::cur_spFGWeightmap.at<float>(pix_pos);

	if( i == 0 )
	{
		if(SalientObjSegmentor::seedMask.at<uchar>(pix_pos) > 0)	// fg
			val = 999999999.9f;
		else
			val = MAX( log(sval/(1-sval)) + SalientObjSegmentor::bias, -800 );
	}
	else	// foreground
	{
		if(SalientObjSegmentor::bgMask.at<uchar>(pix_pos) > 0)	// bg
			val = 99999999999.9f;
		else
			val = 0;	//MAX( -(log(sval/(1-sval)) + SalientObjSegmentor::bias), -800 );
	}

	return val;
}

// pairwise edge cost
MRF::CostVal fnCost(int pix1, int pix2, int i, int j)
{
	MRF::CostVal val = 0;

	Point pix1_pos;
	pix1_pos.x = pix1 % SalientObjSegmentor::width;
	pix1_pos.y = pix1 / SalientObjSegmentor::width;
	Point pix2_pos;
	pix2_pos.x = pix2 % SalientObjSegmentor::width;
	pix2_pos.y = pix2 / SalientObjSegmentor::width;

	// 4-neighbor
	if( abs(pix1_pos.x-pix2_pos.x)+abs(pix1_pos.y-pix2_pos.y)==1 )
	{
		if( i == j )
			val = 0;
		else	// if similar pixels given different labels, penalize more
		{
			// pb dist
		/*	int pb1 = (int)SalientObjSegmentor::norm_img.at<uchar>(pix1_pos);
			int pb2 = (int)SalientObjSegmentor::norm_img.at<uchar>(pix2_pos);
			float dist = abs(pb1-pb2)*1.f/255;*/
			
			Scalar color1 = SalientObjSegmentor::norm_img.at<Vec3f>(pix1_pos);
			Scalar color2 = SalientObjSegmentor::norm_img.at<Vec3f>(pix2_pos);
			float dist = (color1.val[0]-color2.val[0])*(color1.val[0]-color2.val[0]) +\
				(color1.val[1]-color2.val[1])*(color1.val[1]-color2.val[1])+ \
				(color1.val[2]-color2.val[2])*(color1.val[2]-color2.val[2]);
			dist = sqrt(dist/3);
			val = -cv::log(dist)*SalientObjSegmentor::tradeoff;
		}
	}
	else if( pix1 == pix2 )	// same
		val = 0;
	else	// non-adjacent
	{
		/*if( i==j )
			val = INFINITE;
		else
			val = 0;*/
		val = 999999999.9f;
	}

	if(val < 0)
		cout<<val<<endl;

	return val;
}

//////////////////////////////////////////////////////////////////////////
SalientObjSegmentor::SalientObjSegmentor(void)
{
}

SalientObjSegmentor::~SalientObjSegmentor(void)
{
}


bool SalientObjSegmentor::Init(const Mat& img, string save_path, string imgname)
{
	//////////////////////////////////////////////////////////////////////////
	// clear all data and re-initialize
	//////////////////////////////////////////////////////////////////////////
	ClearAll();

	// init params
	width = img.cols;
	height = img.rows;
	numberOfLabels = 2;
	m_patchSize = Point(5, 5);
	m_fSegMinAreaRatio = 0.02;	// 1/50
	m_fSegMaxAreaRatio = 0.5;	// 
	m_fNmsTh = 0.8;
	m_fBGTh = 0.3;
	m_dPropNum = 5;
	m_savePath = save_path;
	m_imgname = imgname;
	// create save directory (if already exist, ignore current image)
	string dirname = m_savePath + m_imgname;
	int ret = mkdir(dirname.c_str());

	// init MRF params
	fgSeedTh = 0.9f;
	bgSeedTh = 0.2f;
	seedMask.create(img.size(), CV_8U);
	bgMask.create(img.size(), CV_8U);
	// standard params from ECCV10 proposal
	int bias_num = 7;
	int tradeoff_num = 8;
	float bia[] = {-2, -1.5, -1, -0.5, 0, 0.5, 1};
	float trad[] = {0, 0.1, 0.5, 1, 2, 5, 10, 20};
	/*float bia[] = {-2, -1.5, -1, -0.5, 0, 0.5, 1};
	float trad[] = {0, 0.05, 0.1, 0.5, 1, 2, 5, 10};*/
	biases.clear();
	for(int i=0; i<bias_num; i++)
		biases.push_back(bia[i]);
	for(int i=0; i<tradeoff_num; i++)
		tradeoffs.push_back(trad[i]);
	
	// color image data for pairwise cost
	norm_img.create(img.size(), CV_32FC3);
	img.convertTo(norm_img, CV_32FC3, 1.f/255);
	// load pb image
	//norm_img.create(img.size(), CV_8U);
	//Mat pimg = imread("D:\\test_samp\\cut\\plan_pb.jpg", 0);
	////cout<<(int)pimg.at<uchar>(155,0)<<endl;
	//resize(pimg, norm_img, norm_img.size());
	//imshow("pb", norm_img);

	// init composition cost computer
	m_CC.Init(img);

	return true;
}

void SalientObjSegmentor::ClearAll()
{
	biases.clear();
	tradeoffs.clear();
	fgWeightmaps.clear();
	spFGWeightmaps.clear();
	seedMask.release();
	norm_img.release();
	segmentRes.clear();
}


void SalientObjSegmentor::ComputePairwiseShortestPathCost(const Mat& img)
{
	// create patch vector
	patchDim = Point(img.cols/m_patchSize.x, img.rows/m_patchSize.y);
	int patchNum = patchDim.x * patchDim.y;
	printf("patch dim: %d, %d\n", patchDim.x, patchDim.y);
	// only need to store useful edges
	vector<vector<pair<int, float>>> patchAdjacencyMatrix;
	// create patches
	m_patches.resize(patchDim.y);
	for(size_t i=0; i<m_patches.size(); i++)
		m_patches[i].resize(patchDim.x);

#pragma region compute patch adjacency matrix

	patchAdjacencyMatrix.resize(patchNum);	// patchNum X patchNum
	for( int y = 0; y < patchDim.y; y++ )
	{
		for( int x = 0; x < patchDim.x; x++)
		{
			// set patch data
			// current patch id (1D) !very important! must consistent with below!
			int id = y*patchDim.x + x;
			m_patches[y][x].id = id;
			m_patches[y][x].box = Rect(x*m_patchSize.x, y*m_patchSize.y, m_patchSize.x, m_patchSize.y);

			// current top-left coordinate
			Point curpos(x*m_patchSize.x, y*m_patchSize.y);

			// compute distance (Euclidean distance of RGB) between neighbors
			// only need right and bottom (forward)
			if( x + 1 < patchDim.x)
			{
				int nextid = y*patchDim.x + (x+1);
				// top left
				Point nextpos((x+1)*m_patchSize.x, y*m_patchSize.y);
				float dist = 0;
				Scalar avg1;
				Scalar avg2;
				for(int yy=0; yy<m_patchSize.y; yy++)
				{
					for(int xx=0; xx<m_patchSize.x; xx++)
					{
						Scalar val1 = img.at<Vec3b>(Point(xx+curpos.x, yy+curpos.y));
						Scalar val2 = img.at<Vec3b>(Point(xx+nextpos.x, yy+nextpos.y));
						for(int k=0; k<3; k++)
						{
							avg1.val[k] += val1.val[k];
							avg2.val[k] += val2.val[k];
						}
						/*for(int id=0; id<3; id++)
							dist += (val1.val[id]-val2.val[id])*(val1.val[id]-val2.val[id]);*/
					}
				}
				for(int k=0; k<3; k++)
				{
					avg1.val[k] /= (m_patchSize.x*m_patchSize.y);
					avg2.val[k] /= (m_patchSize.x*m_patchSize.y);
					dist += (avg1.val[k]-avg2.val[k])*(avg1.val[k]-avg2.val[k]);
				}
				dist = sqrt(dist);
				patchAdjacencyMatrix[id].push_back( make_pair(nextid, dist) );
			}
			if( y + 1 < patchDim.y)
			{
				int nextid = (y+1)*patchDim.x + x;
				Point nextpos( x*m_patchSize.x, (y+1)*m_patchSize.y );
				float dist = 0;
				Scalar avg1;
				Scalar avg2;
				for(int yy=0; yy<m_patchSize.y; yy++)
				{
					for(int xx=0; xx<m_patchSize.x; xx++)
					{
						Scalar val1 = img.at<Vec3b>(Point(xx+curpos.x, yy+curpos.y));
						Scalar val2 = img.at<Vec3b>(Point(xx+nextpos.x, yy+nextpos.y));
						for(int k=0; k<3; k++)
						{
							avg1.val[k] += val1.val[k];
							avg2.val[k] += val2.val[k];
						}
						/*for(int id=0; id<3; id++)
							dist += (val1.val[id]-val2.val[id])*(val1.val[id]-val2.val[id]);*/
					}
				}
				for(int k=0; k<3; k++)
				{
					avg1.val[k] /= (m_patchSize.x*m_patchSize.y);
					avg2.val[k] /= (m_patchSize.x*m_patchSize.y);
					dist += (avg1.val[k]-avg2.val[k])*(avg1.val[k]-avg2.val[k]);
				}
				dist = sqrt(dist);
				patchAdjacencyMatrix[id].push_back( make_pair(nextid, dist) );
			}
		}
	}

#pragma endregion compute patch adjacency matrix

#pragma region compute shortest path cost for all patch pairs

	// prepare to call shortest path algorithm
	// define types
	// patch graph
	typedef adjacency_matrix<undirectedS, no_property, property<edge_weight_t, float>> PGraph;	
	typedef graph_traits<PGraph>::edge_descriptor edge_descriptor;
	typedef graph_traits<PGraph>::vertex_descriptor vertex_descriptor;
	// create graph
	PGraph pg(patchNum);
	property_map<PGraph, edge_weight_t>::type weightmap = get(edge_weight, pg);
	edge_descriptor e; bool inserted;
	for(size_t i=0; i<patchAdjacencyMatrix.size(); i++)
	{
		for(size_t j=0; j<patchAdjacencyMatrix[i].size(); j++)
		{
			boost::tuples::tie(e, inserted) = add_edge(i, patchAdjacencyMatrix[i][j].first, pg);
			weightmap[e] = patchAdjacencyMatrix[i][j].second;
		}
	}
	// set data
	patchDistanceMap.clear();
	patchDistanceMap.resize(patchNum);
	for(size_t i=0; i<patchDistanceMap.size(); i++)
		patchDistanceMap[i].resize(patchNum);

	// call algorithm to compute patch pair-wise shortest path
	johnson_all_pairs_shortest_paths(pg, patchDistanceMap);
	printf("Finish computing shortest paths.\n");

#pragma endregion compute shortest path cost for all patch pairs


}

void SalientObjSegmentor::ComputeFGWeightMap(const Mat& img)
{
	// first compute patch-wise shortest path cost
	ComputePairwiseShortestPathCost(img);

	// set boundary patches (assume background)
	// 0:horizon; 1:vertical; 2:full boundary; 3:boudary except bottom
	vector<vector<int>> boundaryPatches(1);
	patch2boundary.resize(patchDim.y);
	for( size_t y = 0; y < m_patches.size(); y++ )
	{
		patch2boundary[y].resize(patchDim.x);
		for( size_t x = 0; x < m_patches[y].size(); x++)
		{
			int id = m_patches[y][x].id;
			// set boundary patches (only consider 2 now)
			/*if(y==0 || y==m_patches.size()-1)
				boundaryPatches[0].push_back(id);
			if(x==0 || x==m_patches[0].size()-1)
				boundaryPatches[1].push_back(id);*/
			if(y==0 || x==0 || x==m_patches[0].size()-1 || y==m_patches.size()-1)
				boundaryPatches[0].push_back(id);
			/*if(y==0 || x==0 || x==m_patches[0].size()-1)
				boundaryPatches[3].push_back(id);*/
		}
	}

#pragma region compute fg weight

	// compute background likelihood map
	fgWeightmaps.resize(1);
	for(size_t i=0; i<fgWeightmaps.size(); i++)
	{
		fgWeightmaps[i].create(img.size(), CV_32F);
		fgWeightmaps[i].setTo(0);
	}
	// loop each patch
	for(size_t i=0; i<patchDistanceMap.size(); i++)
	{
		int y = i/patchDim.x;
		int x = i%patchDim.x;
		// loop each background setting
		for(size_t k=0; k<boundaryPatches.size(); k++)
		{
			float minDist = 999999999.9f;
			for(size_t j=0; j<boundaryPatches[k].size(); j++)
			{
				if(patchDistanceMap[i][boundaryPatches[k][j]] < minDist)
					minDist = patchDistanceMap[i][boundaryPatches[k][j]];
			}
			m_patches[y][x].bgScore = minDist;
			patch2boundary[y][x] = minDist;
			// fill weight map
			fgWeightmaps[k](m_patches[y][x].box).setTo(minDist);
		}
	}
	for(size_t i=0; i<fgWeightmaps.size(); i++)
	{
		cv::normalize(fgWeightmaps[i], fgWeightmaps[i], 1, 0, NORM_INF);
	}
	
	// compute importance factor for each superpixel
	const vector<Superpixel>& superpixels = m_CC.m_segmentor.superpixels;
	vector<SegSuperPixelFeature>& sp_features = m_CC.sp_features;
	float maxSPWeight = 0;
	spFGWeightmaps.resize(1);
	for(size_t k=0; k<fgWeightmaps.size(); k++)
	{
		spFGWeightmaps[k].create(img.size(), CV_32F);
		spFGWeightmaps[k].setTo(0);
		for(size_t i=0; i<superpixels.size(); i++)
		{
			const Superpixel& cursp = superpixels[i];
			SegSuperPixelFeature& cursp_feat = sp_features[i];
			cursp_feat.fgWeights.resize(fgWeightmaps.size(), 0);
			// compute average pixel foreground weight within current superpixel
			cursp_feat.fgWeights[k] = mean(fgWeightmaps[k], cursp.sp_mask).val[0];
			if(cursp_feat.fgWeights[k] > maxSPWeight)
				maxSPWeight = cursp_feat.fgWeights[k];

			// fill weight map
			spFGWeightmaps[k].setTo(cursp_feat.fgWeights[k], cursp.sp_mask);
		}

		// normalize superpixel fg weight (max = 1)
		for(size_t i=0; i<sp_features.size(); i++)
		{
			sp_features[i].fgWeights[k] /= maxSPWeight;
		}
		// normalize superpixel fg weight map
		cv::normalize(spFGWeightmaps[k], spFGWeightmaps[k], 1, 0, NORM_INF);
	}

#pragma endregion compute fg weight

	// show weight image
	char str[100];
	for(size_t i=0; i<fgWeightmaps.size(); i++)
	{
		Mat showimg(fgWeightmaps[i].size(), CV_8U);
		fgWeightmaps[i].convertTo(showimg, CV_8U, 255);
		sprintf(str, "pixel_map_%d", i);
		imshow(str, showimg);
		spFGWeightmaps[i].convertTo(showimg, CV_8U, 255);
		sprintf(str, "sp_map_%d", i);
		imshow(str, showimg);
	}
	waitKey(10);
}

void SalientObjSegmentor::RunSegmentation(const Mat& img)
{
	// compute saliency map
	ComputeFGWeightMap(img);

#pragma region segmentation

	// set up MRF
	// set data term
	DataCost *data = new DataCost( dCost );
	// set smooth term
	SmoothnessCost *smooth = new SmoothnessCost( fnCost );
	// set energy function
	EnergyFunction *eng = new EnergyFunction(data, smooth);

	float t;
	MRF *mrf = new Expansion(width, height, numberOfLabels, eng);
	mrf->initialize();

	// get necessary data
	const vector<SegSuperPixelFeature>& sp_features = m_CC.sp_features;
	const vector<Superpixel>& superpixels = m_CC.m_segmentor.superpixels;
	// set result data
	Mat segmentMask(img.size(), CV_8U);	// binary segment image
	Mat objImg(img.size(), CV_8UC3);	// cropped segment image
	Mat labelMat(img.size(), CV_8U);
	segmentRes.clear();
	segmentRes.reserve(biases.size()*tradeoffs.size()*superpixels.size()/2);

	// set bg mask (same for all seeds)
	bgMask.setTo(0);
	for(size_t id=0; id<sp_features.size(); id++)
	{
		if(sp_features[id].fgWeights[0] < bgSeedTh)
			bgMask.setTo(255, superpixels[id].sp_mask);
	}
	imshow("sp bg mask", bgMask);
	waitKey(10);

	// fg
	cur_spFGWeightmap.release();
	cur_spFGWeightmap.create(img.size(), CV_32F);
	// directly use fg weight map as unified data map
	//spFGWeightmaps[0].copyTo(cur_spFGWeightmap);

	// grid sampling
	assert(!m_patches.empty());
	int samplingStep = 8;
	Point steps(m_patches[0].size()/samplingStep, m_patches.size()/samplingStep);
	/*for(size_t y=0; y<m_patches.size(); y+=steps.y)
	{
	for(size_t x=0; x<m_patches[y].size(); x+=steps.x)
	{*/
			// select seed patch
			const Patch& seed = m_patches[3][3];
			// use superpixel covering the patch as seeds
			int bestId = -1;
			float max_overlap = 0;
			for(size_t k=0; k<superpixels.size(); k++)
			{
				int overlap = countNonZero(superpixels[k].sp_mask(seed.box));
				if( overlap > max_overlap )
				{
					max_overlap = overlap;
					bestId = k;
				}
			}
			// skip background sp
			/*if(sp_features[bestId].fgWeights[0] < bgSeedTh)
			continue;*/
			// set current seed mask
			seedMask.setTo(0);
			seedMask.setTo(255, superpixels[bestId].sp_mask);
			imshow("sp seed mask", seedMask);
			waitKey(10);

			// compute weight map as ratio of cost to seed and cost to boundary
			cur_spFGWeightmap.setTo(0);
			for(size_t y=0; y<m_patches.size(); y++)
			{
				for(size_t x=0; x<m_patches[y].size(); x++)
				{
					int curid = m_patches[y][x].id;
					cur_spFGWeightmap(m_patches[y][x].box).setTo(patchDistanceMap[curid][seed.id]);
				}
			}
			normalize(cur_spFGWeightmap, cur_spFGWeightmap, 1, 0, NORM_INF);
			subtract(1, cur_spFGWeightmap, cur_spFGWeightmap);
			imshow("data map", cur_spFGWeightmap);
			waitKey(10);

			// loop params to do graph-cut
			for(size_t i=0; i<biases.size(); i++)
			{
				for(size_t j=0; j<tradeoffs.size(); j++)
				{
					// temporal result
					SegmentResult curResult;
					// set params
					bias = biases[i];
					tradeoff = tradeoffs[j];
					curResult.bias = bias;
					curResult.tradeoff = tradeoff;
					cout<<"Segment using "<<bias<<" and "<<tradeoff<<endl;

					// solve
					mrf->clearAnswer();
					mrf->optimize(1, t);
					MRF::EnergyVal E_smooth = mrf->smoothnessEnergy();
					MRF::EnergyVal E_data   = mrf->dataEnergy();
					curResult.data_energy = E_data;
					curResult.smooth_energy = E_smooth;
					curResult.total_energy = E_smooth + E_data;
					printf("Total Energy = %f (Smoothness energy %f, Data Energy %f)\n", E_smooth+E_data,E_smooth,E_data);

					// set labels
					segmentMask.setTo(0);
					labelMat.setTo(0);
					for (int pix = 0; pix < width*height; pix++ )
					{
						Point pos;
						pos.x = pix % width;
						pos.y = pix / width;
						labelMat.at<uchar>(pos) = mrf->getLabel(pix);
					}

					// show results
					labelMat.convertTo(segmentMask, CV_8U, 255);
					imshow("mask", segmentMask);
					waitKey(10);
					// filter mask to keep biggest one only (may change later)
					vector<vector<Point>> contours;
					Mat mask_backup(segmentMask.size(), CV_8U);	// changed
					segmentMask.copyTo(mask_backup);
					findContours(mask_backup, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
					double maxContourArea = 0;
					int maxContourId = -1;
					for(size_t i=0; i<contours.size(); i++)
					{
						double area = contourArea( Mat(contours[i]) );
						if(area > maxContourArea)
						{
							maxContourArea = area;
							maxContourId = i;
						}
					}
					// too small or too big
					float imageArea = img.rows*img.cols;
					if(maxContourArea < m_fSegMinAreaRatio*imageArea || \
						maxContourArea > m_fSegMaxAreaRatio*imageArea)
						continue;

					// fill all non-max contour (remove)
					for(size_t i=0; i<contours.size(); i++)
					{
						if( i != maxContourId )
						{
							// compute convex hull
							vector<Point> conv_hull;
							convexHull( Mat(contours[i]), conv_hull );
							fillConvexPoly(segmentMask, &conv_hull[0], conv_hull.size(), CV_RGB(0,0,0));
						}
					}

					// copy image part of mask
					objImg.setTo(255);
					img.copyTo(objImg, segmentMask);
					// save to results
					curResult.mask.create(segmentMask.size(), CV_8U);
					segmentMask.copyTo(curResult.mask);
					curResult.area = countNonZero(curResult.mask);
					curResult.obj_img.create(objImg.size(), CV_8UC3);
					objImg.copyTo(curResult.obj_img);
					// show
					imshow("mask", segmentMask);
					imshow("obj", objImg);
					waitKey(10);

					// add to total results
					segmentRes.push_back(curResult);
				}
			}
			/*	}
			}*/
	
	// release mrf data
	delete mrf;
	delete data;
	delete smooth;
	delete eng;

#pragma endregion segmentation
	
#pragma region rank

	// compute composition cost for all segment candidates
	for(size_t i=0; i<segmentRes.size(); i++)
	{
		segmentRes[i].CC_score = m_CC.ComputeCompositionCost(segmentRes[i].mask);
	}
	// rank results by CC score (descending)
	sort( segmentRes.begin(), segmentRes.end(), SalientObjSegmentor::rank_by_cc );
	
#pragma endregion rank

#pragma region do segment nms

	// remove overlapped segments with lower score
	//Mat finalMask(img.size(), CV_8U);
	//for(size_t i=0; i<segmentRes.size(); i++)	// from high score to low
	//{
	//	if( !segmentRes[i].m_bValid )
	//		continue;

	//	for(size_t j=i+1; j<segmentRes.size(); j++)
	//	{
	//		if( !segmentRes[j].m_bValid )
	//			continue;

	//		// compute intersection/union
	//		float intersectArea = 0;
	//		float unionArea = 0;
	//		// intersection
	//		bitwise_and( segmentRes[i].mask, segmentRes[j].mask, finalMask );
	//		intersectArea = countNonZero(finalMask);
	//		// union
	//		bitwise_or( segmentRes[i].mask, segmentRes[j].mask, finalMask );
	//		unionArea = countNonZero(finalMask);
	//		if(intersectArea / unionArea > m_fNmsTh)	// remove low score segment
	//			segmentRes[j].m_bValid = false;
	//	}
	//}

#pragma endregion do segment nms
	
#pragma region save results

	// save to files
	char str[100];
	// foreground weight map
	for(size_t k=0; k<fgWeightmaps.size(); k++)
	{
		// foreground weight map
		Mat vis_img(fgWeightmaps[k].size(), CV_8U);
		sprintf(str, "%d", k);
		string pixwmap_file = m_savePath + m_imgname + "\\pix_wmap" + str + ".jpg";
		fgWeightmaps[k].convertTo(vis_img, CV_8U, 255);
		imwrite(pixwmap_file.c_str(), vis_img);
		string spwmap_file = m_savePath + m_imgname + "\\sp_wmap" + str + ".jpg";
		spFGWeightmaps[k].convertTo(vis_img, CV_8U, 255);
		imwrite(spwmap_file.c_str(), vis_img);
	}

	// save useful results to a big image
	// image seg
	// res1   res2	res3...
	// the rest results are saved to individual files
	int showResNum = 5;
	Mat wholeResult(img.rows*2, img.cols*showResNum, CV_8UC3);
	wholeResult.setTo( 0 );
	Rect roi;
	// image
	roi = Rect(0, 0, img.cols, img.rows);
	img.copyTo( wholeResult(roi) );
	// superpixel map
	roi.x = img.cols;
	roi.y = 0;
	m_CC.m_segmentor.m_segImg.copyTo( wholeResult(roi) );
	//string sp_file = m_savePath + m_imgname + "\\sp.jpg";
	//imwrite(sp_file.c_str(), m_CC.m_segmentor.m_segImg);

	// save ranked segmentation results
	int cnt = -1;
	for(size_t i=0; i<segmentRes.size(); i++)
	{
		if( segmentRes[i].m_bValid )
		{
			cnt++;
			const SegmentResult& cur_res = segmentRes[i];
			if( cnt < showResNum )
			{
				roi.x = img.cols*cnt;
				roi.y = img.rows;
				cur_res.obj_img.copyTo( wholeResult(roi) );
				//continue;
			}

			// save path format: rank_id+CC_score+total_en+bias+tradeoff+type
			// save mask: imagename + id
			string maskPath;
			maskPath = m_savePath + m_imgname + "\\" + m_imgname + "_%d_mask.jpg";
			sprintf(str, maskPath.c_str(), cnt);
			imwrite(str, cur_res.mask);
			// save object
			string objPath = m_savePath + m_imgname + "\\%3d_cc_%.2f_ene_%.2f_bias_%.2f_to_%.2f_obj.jpg";
			sprintf(str, objPath.c_str(), cnt, cur_res.CC_score, cur_res.total_energy, cur_res.bias, cur_res.tradeoff);
			imwrite(str, cur_res.obj_img);
		}
	}

	// save whole result
	string bigResPath = m_savePath + m_imgname + "_whole_res.jpg";
	imwrite(bigResPath.c_str(), wholeResult);

#pragma endregion save results
	
	cout<<"Finish processing."<<endl;

}