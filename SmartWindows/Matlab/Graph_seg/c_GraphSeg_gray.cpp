#include "StdAfx.h"
#include "c_GraphSeg_gray.h"

c_GraphSeg_gray::c_GraphSeg_gray(const double *p_img_in, 
								 const int imgWidth, const int imgHeight,
								 const int neighbor_radius, const double threshold,
								 const int min_RegionSize, const int num_channels)
{
	m_pImg_in = p_img_in;
	m_imgWidth = imgWidth;
	m_imgHeight = imgHeight;
	m_num_pixels = m_imgWidth * m_imgHeight;
	m_neighbor_radius = neighbor_radius;
	m_threshold = threshold;
	m_minRegion_size = min_RegionSize;
	m_num_channels = num_channels;

	m_pImg_out = NULL;
	//Initialize Edge set:
	Extract_edges Extract_edges_obj(p_img_in, m_imgHeight, m_imgWidth, m_neighbor_radius,
		                            m_num_channels);
	Extract_edges_obj.find_edges();
	m_num_edges = Extract_edges_obj.m_num_edges;
	m_pEdge_set = new Edge[m_num_edges];

	memcpy(m_pEdge_set, Extract_edges_obj.m_edges, m_num_edges*sizeof(Edge));
	sorting_edges(m_pEdge_set, m_num_edges);
}

c_GraphSeg_gray::~c_GraphSeg_gray()
{
	if(m_pImg_out)
	{
		delete [] m_pImg_out;
	}
	delete [] m_pEdge_set;
}

void c_GraphSeg_gray::segment()
{
	c_GraphSeg c_GraphSeg_obj(m_pEdge_set, m_num_edges,
		                      m_imgWidth, m_imgHeight,
							  m_threshold, m_minRegion_size);
	c_GraphSeg_obj.segment();

	m_pImg_out = new double[m_num_pixels];
	memcpy(m_pImg_out, c_GraphSeg_obj.m_pImg_out, m_num_pixels*sizeof(double));
}
