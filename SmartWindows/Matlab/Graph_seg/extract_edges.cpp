#include "StdAfx.h"
#include "extract_edges.h"
#include "public_functions_lib.h"

Extract_edges::Extract_edges(const double *p_img,
							 int img_height, int img_width, int neighbor_radius,
							 const int num_channels):m_sigma(1)
{

	m_pImg = p_img;
	m_ImgHeight = img_height;
	m_ImgWidth = img_width;
	m_num_vetex = m_ImgHeight * m_ImgWidth;
	m_num_channels = num_channels;

	m_neighbor_radius = neighbor_radius;
	m_edges = NULL;
	
	m_num_edges = 0;
}

Extract_edges::Extract_edges(const double *p_img,
							 int img_height, int img_width, 
							 int neighbor_radius, const double sigma,
							 const int num_channels):m_sigma(sigma)
{

	m_pImg = p_img;
	m_ImgHeight = img_height;
	m_ImgWidth = img_width;
	m_num_vetex = m_ImgHeight * m_ImgWidth;
	m_num_channels = num_channels;

	m_neighbor_radius = neighbor_radius;
	m_edges = NULL;
	
	m_num_edges = 0;
}


Extract_edges::~Extract_edges()
{
	if(m_edges)
	{
		delete [] m_edges;
	}
}

void Extract_edges::find_edges()
{
	int num_edges_max = ( (2*m_neighbor_radius+1)*(2*m_neighbor_radius+1)-1 )*m_num_vetex;
	m_edges = new Edge[num_edges_max];
	assert(m_edges);

	int x, y, nx_idx, ny_idx, nx, ny, p, q;
	//for each pixel p in the image
    for(x=0; x<m_ImgWidth; x++)
    {
        for(y=0; y<m_ImgHeight; y++)
        {
            p=y+m_ImgHeight*x;
            //for each neighbouring q of p
            for(nx_idx=-m_neighbor_radius; nx_idx<=m_neighbor_radius; nx_idx++)
            {
                for(ny_idx=-m_neighbor_radius; ny_idx<=m_neighbor_radius; ny_idx++)
                {
                    nx=x+nx_idx;
                    ny=y+ny_idx;
                    q=ny+m_ImgHeight*nx;
                    if( InRange(nx, ny, m_ImgWidth, m_ImgHeight) )
                    {
                        if(q==p) continue;
                        m_edges[m_num_edges].a = p;
                        m_edges[m_num_edges].b = q;
                        m_edges[m_num_edges].w = pixelDistance_3d(m_pImg, p, q, 
							                                      m_num_channels, m_num_vetex);
                        m_num_edges++;
                    }
                }
            }
        }
    }
}

void Extract_edges::find_edges_power()
{
	int num_edges_max = ( (2*m_neighbor_radius+1)*(2*m_neighbor_radius+1)-1 )*m_num_vetex;
	m_edges = new Edge[num_edges_max];
	assert(m_edges);

	int x, y, nx_idx, ny_idx, nx, ny, p, q;
	//for each pixel p in the image
    for(x=0; x<m_ImgWidth; x++)
    {
        for(y=0; y<m_ImgHeight; y++)
        {
            p=y+m_ImgHeight*x;
            //for each neighbouring q of p
            for(nx_idx=-m_neighbor_radius; nx_idx<=m_neighbor_radius; nx_idx++)
            {
                for(ny_idx=-m_neighbor_radius; ny_idx<=m_neighbor_radius; ny_idx++)
                {
                    nx=x+nx_idx;
                    ny=y+ny_idx;
                    q=ny+m_ImgHeight*nx;
                    if( InRange(nx, ny, m_ImgWidth, m_ImgHeight) )
                    {
                        if(q==p) continue;
                        m_edges[m_num_edges].a = p;
                        m_edges[m_num_edges].b = q;
                        m_edges[m_num_edges].w = pixelDistance_3d(m_pImg, p, q, 
							                                      m_num_channels, m_num_vetex,
																  m_ImgHeight, m_sigma);
                        m_num_edges++;
                    }
                }
            }
        }
    }
}