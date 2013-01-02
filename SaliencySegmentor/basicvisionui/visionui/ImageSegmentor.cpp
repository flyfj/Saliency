//////////////////////////////////////////////////////////////////////////


#include "ImageSegmentor.h"


ImageSegmentor::ImageSegmentor(void)
{
	m_dSmoothSigma = 0.5f;
	m_dThresholdK = 80.f;
	m_dMinArea = 100;
}

ImageSegmentor::~ImageSegmentor(void)
{
}


int ImageSegmentor::DoSegmentation(const Mat& img)
{
	//do segmentation
	return graph_based_segment(img, m_dSmoothSigma, m_dThresholdK, m_dMinArea, m_idxImg, m_segImg);

}