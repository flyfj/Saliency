#ifndef EXTRACT_EDGES_H
#define EXTRACT_EDGES_H

#include "Edge.h"
class Extract_edges
{
public:
	//methods:
	Extract_edges(const double *p_img,
		int img_height, int img_width, int neighbor_radius,
		const int num_channels);
	Extract_edges(const double *p_img,
		int img_height, int img_width, 
		int neighbor_radius, const double sigma,
		const int num_channels);

	~Extract_edges();
	void find_edges();
	void find_edges_power();
public:
	const double *m_pImg;
	Edge *m_edges;
	int m_num_edges;

	int m_ImgHeight;
	int m_ImgWidth;
	int m_num_vetex;
	int m_neighbor_radius;
	int m_num_channels;
	const double m_sigma;
};
#endif