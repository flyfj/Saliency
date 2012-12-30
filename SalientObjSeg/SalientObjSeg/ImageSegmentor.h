//////////////////////////////////////////////////////////////////////////
//	superpixel segmentation
//	fengjie@cis.pku
//	2011/9/6
//////////////////////////////////////////////////////////////////////////


#pragma once

#include <opencv2\opencv.hpp>
#include <vector>
using namespace cv;


// information of another segment
struct TPair
{
	TPair(): id(-1), appdist(0), spadist(0), saliency(0){}

	int id;
	float appdist;
	float spadist;
	float saliency;

	static bool comp_by_saliency(const TPair& a, const TPair& b) 
	{
		return a.saliency < b.saliency;	
	}
};

typedef struct Superpixel
{
	//data
	Mat sp_mask;
	int area;
	Rect bbox;
	Point2f centroid;
	bool ifBoundarySeg;
};


class ImageSegmentor
{
public:
	//segment-related
	vector<Superpixel> superpixels;	//integral image for each segment
	float m_dSmoothSigma;
	float m_dThresholdK;
	int m_dMinArea;
	Mat m_segImg;
	Mat m_idxImg;	// superpixel index

public:
	ImageSegmentor(void);
	~ImageSegmentor(void);

	void DoSegmentation(const Mat& img);
};
