// Graph_seg_gray.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Graph_seg_gray.h"

#include "mex.h"    //MATLAB API
#include "c_GraphSeg_gray.h"
/*
#pragma comment(lib, "libmx.lib")
#pragma comment(lib, "libmat.lib")
#pragma comment(lib, "libmex.lib")

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


// This is an example of an exported variable
GRAPH_SEG_GRAY_API int nGraph_seg_gray=0;

// This is an example of an exported function.
GRAPH_SEG_GRAY_API int fnGraph_seg_gray(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see Graph_seg_gray.h for the class definition
CGraph_seg_gray::CGraph_seg_gray()
{ 
	return; 
}
*/
//Inputs:
//in_args[0] --> double image
//in_args[1] --> neighbor_radius
//in_args[2] --> threshold
//in_args[3] --> min_Region_size


//Outputs:
//out_args[0] --> labeled_image

void mexFunction(
				 //outputs:
				 int n_outs, mxArray *out_args[],
				 //inputs:
				 const int n_ins, const mxArray *in_args[])
{
	//import data:
	const double *pImg_in = mxGetPr(in_args[0]);
	const int *pImg_demension = mxGetDimensions(in_args[0]);
	
	const int imgWidth = pImg_demension[1];
	const int imgHeight = pImg_demension[0];
	const int num_pixels = imgWidth * imgHeight;
	const int num_channels = mxGetNumberOfElements(in_args[0]) / num_pixels;

	const int neighbor_radius = (int)mxGetScalar(in_args[1]);
	const double threshold = mxGetScalar(in_args[2]);
	const int min_region_size = (int)mxGetScalar(in_args[3]);
	mexPrintf("%d, %d, %d\n", imgWidth, imgHeight, num_channels);

	//Do main computation:
	c_GraphSeg_gray c_GraphSeg_gray_obj(pImg_in, imgWidth, imgHeight, 
		                                neighbor_radius, threshold,
										min_region_size, num_channels);
	c_GraphSeg_gray_obj.segment();

	//construct outputs:
	out_args[0] = mxCreateDoubleMatrix(imgHeight, imgWidth, mxREAL);
	double *pImg_out = mxGetPr(out_args[0]);

	memcpy(pImg_out, c_GraphSeg_gray_obj.m_pImg_out, imgHeight*imgWidth*sizeof(double));
}