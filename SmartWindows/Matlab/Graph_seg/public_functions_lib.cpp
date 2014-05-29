#include "StdAfx.h"
#include "public_functions_lib.h"

double pixelDistance(const double *p_img, const int p, int q)
{
	double p_intensity, q_intensity, intensity_Distance;
	p_intensity = p_img[p];
	q_intensity = p_img[q];
    intensity_Distance = p_intensity-q_intensity;
    if(intensity_Distance<0)
    {
        intensity_Distance = -intensity_Distance;
    }
	return intensity_Distance;
}

double pixelDistance(const double *p_img, const int p, int q,
					 const double distance, const double sigma)
{
	double p_intensity, q_intensity, intensity_Distance;
	p_intensity = p_img[p];
	q_intensity = p_img[q];
    intensity_Distance = p_intensity-q_intensity;
    if(intensity_Distance<0)
    {
        intensity_Distance = -intensity_Distance;
    }
	intensity_Distance *= exp(-(distance-1) / sigma);
	return intensity_Distance;
}

double pixelDistance_3d(const double *p_img, const int p, int q, 
						const int n_channels, const num_pixels)
{
	double intensity_distance = 0;
	int i;
	double d;
	for(i=0; i<n_channels; i++)
	{
		d = pixelDistance(p_img+i*num_pixels, p, q);
		if(d > intensity_distance)
		{
			intensity_distance = d;
		}
	}
	return intensity_distance;
}

double pixelDistance_3d(const double *p_img, const int p, int q, 
						const int n_channels, const num_pixels,
						const int Img_height, const double sigma)
{
	int p_x = p/Img_height;
	int p_y = p-p_x*Img_height;

	int q_x = q/Img_height;
	int q_y = q-q_x*Img_height;

	double distance = (p_x-q_x)*(p_x-q_x) + (p_y-q_y)*(p_y-q_y);
//compute distance:
	double intensity_distance = 0;
	int i;
	double d;
	for(i=0; i<n_channels; i++)
	{
		d = pixelDistance(p_img+i*num_pixels, p, q, distance, sigma);
		if(d > intensity_distance)
		{
			intensity_distance = d;
		}
	}
	return intensity_distance;
}

bool InRange(const int x, const int y, 
			 const int ImgWidth, const int ImgHeight)
{
	bool flag_x, flag_y;
	flag_x = (x>=0 && x<ImgWidth);
	flag_y = (y>=0 && y<ImgHeight);
	return (flag_x && flag_y);
}

double double_abs(const double x)
{
	if(x<0)
	{
		return -x;
	}
	else
	{
		return x;
	}
}

void print_edges(Edge *p_edges, int num_edges)
{
	int i;
	for(i=0; i<num_edges; i++)
	{
		cout<<"["<<p_edges[i].a<<", "<<p_edges[i].b<<", "<<p_edges[i].w<<"]"
			<<"\t";
	}
	cout<<endl;
}

//files IO between matlab and vc
void read_2d_matrix(
					//inputs:
					const char *file_name,
					//outputs:
		            double **pp_img, int *p_imgWidth, int *p_imgHeight)
{
	fstream file_reader;
	file_reader.open(file_name, ios::in | ios::binary);
	//figuer out the size of the matrix;
	//the array structure is:
	//[length, imgWidth, imgHeight, array_elements];
	double header[3];
	file_reader.read(reinterpret_cast<char *>(&header[0]), 3*sizeof(double));
	file_reader.seekg(ios_base::beg);

	const int file_size = (int)header[0];
	(*p_imgHeight) = (int)header[1];
	(*p_imgWidth) = (int)header[2];
	int num_pixels = (*p_imgHeight) * (*p_imgWidth);

	double *p_data_inFile = new double[file_size];
	file_reader.read(reinterpret_cast<char *>(p_data_inFile), file_size*sizeof(double));
	(*pp_img) = new double[num_pixels];
	memcpy((*pp_img), p_data_inFile+3, num_pixels*sizeof(double));
	//free up memories:
	delete [] p_data_inFile;
}

void print_2d_matrix(const double *p_img, const int imgWidth, const int imgHeight)
{
	int i, j, idx;
	for(i=0; i<imgHeight; i++)
	{
		for(j=0; j<imgWidth; j++)
		{
			idx = i + j*imgHeight;
			cout<<p_img[idx]<<"\t";
		}
		cout<<endl;
	}

}

bool check_onBorder(const int idx, const int imgWidth, const int imgHeight)
{
	int idx_W = idx/imgHeight;
	int idx_H = idx - idx_W*imgHeight;

	bool flag = (idx_W==0 || idx_W==imgWidth-1) || 
		(idx_H==0 || idx_H==imgHeight-1);
	return flag;
}

bool edges_compare_func(Edge Edge_obj1, Edge Edge_obj2)
{
	return Edge_obj1.w < Edge_obj2.w;
}

void sorting_edges(Edge *pEdges_set, int num_edges)
{

	//sort(pEdges_vector->begin(), pEdges_vector->end(), &edges_compare_func);
	
	vector<Edge> edges_vector(pEdges_set, pEdges_set+num_edges);
	sort(edges_vector.begin(), edges_vector.end(), &edges_compare_func);

	int i;
	for(i=0; i<num_edges; i++)
	{
		pEdges_set[i].a = edges_vector[i].a;
		pEdges_set[i].b = edges_vector[i].b;
		pEdges_set[i].w = edges_vector[i].w;
	}
	edges_vector.clear();
}

bool operator< (const Edge Edge_elt1, const Edge Edge_elt2)
{
	bool flag = false;
	if(Edge_elt1.a < Edge_elt2.a)
	{
		flag = true;
	}
	else if(Edge_elt1.a == Edge_elt2.a)
	{
		flag = (Edge_elt1.b < Edge_elt2.b); 
	}

	return flag;
}
