#include "StdAfx.h"
#include "c_GraphSeg.h"


c_GraphSeg::c_GraphSeg(const Edge *pEdge_set, const int num_edges,
					   const int imgWidth, const int imgHeight,
					   const double threshold, const int min_RegionSize)
{
	m_pEdge_set = pEdge_set;
	m_num_edges = num_edges;
	m_imgWidth = imgWidth;
	m_imgHeight = imgHeight;
	m_num_pixels = imgWidth * imgHeight;

	m_threshold = threshold;
	m_minRegion_size = min_RegionSize;

	//initialize region threshold
	m_pThreshold = new double[m_num_pixels];
	m_pSize = new double[m_num_pixels];
	m_pImg_out = new double[m_num_pixels];
	int i;
	for(i=0; i<m_num_pixels; i++)
	{
		m_pThreshold[i] = m_threshold;
		m_pSize[i] = 1;
		m_pImg_out[i] = 0;
	}
}

c_GraphSeg::~c_GraphSeg()
{
	delete [] m_pImg_out;
	delete [] m_pThreshold;
	delete [] m_pSize;
}

void c_GraphSeg::merge()
{
	Disjoint_set Disjoint_set_obj(m_num_pixels);
	int pixel_a, pixel_b;
	int region_a, region_b, parent_region;
	double edge_weight;
	int i;

	for(i=0; i<m_num_edges; i++)
	{
		pixel_a = m_pEdge_set[i].a;
		pixel_b = m_pEdge_set[i].b;
		edge_weight = m_pEdge_set[i].w;
		
		region_a = Disjoint_set_obj.find(pixel_a);
		region_b = Disjoint_set_obj.find(pixel_b);

		if( region_a != region_b &&
			edge_weight < m_pThreshold[region_a] &&
			edge_weight < m_pThreshold[region_b])
		{
			parent_region = Disjoint_set_obj.joint(pixel_a, pixel_b);
			m_pSize[parent_region] = m_pSize[region_a] + m_pSize[region_b];
			m_pThreshold[parent_region] = edge_weight + m_threshold/double(m_pSize[parent_region]);
		}
	}

	//Removing grain area:
	Remove_grains(&Disjoint_set_obj);
	//label:
	label(&Disjoint_set_obj);
}

void c_GraphSeg::label(Disjoint_set *p_Disjoint_set)
{
	int p;
	for(p=0; p<m_num_pixels; p++)
	{
		m_pImg_out[p] = p_Disjoint_set->find(p);
	}
}

void c_GraphSeg::Remove_grains(Disjoint_set *pDisjoint_set_obj)
{
	int pixel_a, pixel_b;
	int region_a, region_b, parent_region;
	double edge_weight;
	int i;

	for(i=0; i<m_num_edges; i++)
	{
		pixel_a = m_pEdge_set[i].a;
		pixel_b = m_pEdge_set[i].b;
		edge_weight = m_pEdge_set[i].w;
		
		region_a = pDisjoint_set_obj->find(pixel_a);
		region_b = pDisjoint_set_obj->find(pixel_b);

		if( region_a != region_b &&
			(m_pSize[region_a] < m_minRegion_size ||
			m_pSize[region_b] < m_minRegion_size) )
		{
			parent_region = pDisjoint_set_obj->joint(pixel_a, pixel_b);
			m_pSize[parent_region] = m_pSize[region_a] + m_pSize[region_b];
		}
	}
}
void c_GraphSeg::segment()
{
	merge();
}