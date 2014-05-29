#ifndef C_GRAPHSEG_GRAY_H
#define C_GRAPHSEG_GRAY_H

#include "public_functions_lib.h"
#include "extract_edges.h"
#include "c_GraphSeg.h"

class c_GraphSeg_gray
{
public:
	//methods:
	c_GraphSeg_gray(const double *p_img_in, const int imgWidth, const int imgHeight,
		            const int neighbor_radius, const double threshold, 
					const int min_RegionSize, const int num_channels);
	~c_GraphSeg_gray();

	void segment();

public:
	//members:
	int m_imgWidth;
	int m_imgHeight;
	int m_num_pixels;

	int m_neighbor_radius;
	double m_threshold;
	int m_minRegion_size;
	int m_num_channels;

	int m_num_edges;

	const double *m_pImg_in;
	double *m_pImg_out;
	Edge *m_pEdge_set;
};

#endif