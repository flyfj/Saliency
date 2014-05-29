#ifndef C_GRAPH_SEG_H
#define C_GRAPH_SEG_H
#include "Edge.h"
#include "Disjoint_set.h"

class
c_GraphSeg
{
public:
	//methods:
	c_GraphSeg(const Edge *pEdge_set, const int num_edges,
		const int imgWidth, const int imgHeight,
		const double threshold, const int min_Region_size);
	~c_GraphSeg();

	void segment();
	void merge();
	void label(Disjoint_set *p_Disjoint_set);
	void Remove_grains(Disjoint_set *p_Disjoint_set);

public:
	//members:
	const Edge *m_pEdge_set;
	int m_num_edges;
	int m_imgWidth, m_imgHeight, m_num_pixels;
	double m_threshold;
	int m_minRegion_size;

	double *m_pThreshold;
	double *m_pSize;
	double *m_pImg_out;
};

#endif