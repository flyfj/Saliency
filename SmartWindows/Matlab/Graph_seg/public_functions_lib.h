#ifndef PUBLIC_FUNCTIONS_LIB_H
#define PUBLIC_FUNCTIONS_LIB_H

#include <math.h>
#include "extract_edges.h"
#include "Edge.h"
#include <vector>
#include <algorithm>

using namespace std;

const double DOUBLE_EPS = 1E-15;

double pixelDistance(const double *p_img, const int p, int q);
double pixelDistance(const double *p_img, const int p, int q,
					 const double distance, const double sigma);

double pixelDistance_3d(const double *p_img, const int p, int q, 
						const int n_channels, const num_pixels);
double pixelDistance_3d(const double *p_img, const int p, int q, 
						const int n_channels, const num_pixels,
						const int Img_height, const double sigma);

bool InRange(const int x, const int y, 
			 const int ImgWidth, const int ImgHeight);

//files IO between matlab and vc
void read_2d_matrix(
					//inputs:
					const char *file_directory,
					//outputs:
		            double **pp_img, int *p_imgWidth, int *p_imgHeight);

void print_2d_matrix(const double *p_img, const int imgWidth, const int imgHeight);

double double_abs(const double x);

void print_edges(Edge *p_edges, int num_edges);

bool check_onBorder(const int idx, const int imgWidth, const int imgHeight);

//sort edges:
bool edges_compare_func(Edge, Edge);
void sorting_edges(Edge *pEdges_set, int num_edges);

bool operator< (const Edge Edge_elt1, const Edge Edge_elt2);
#endif