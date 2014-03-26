
#include <opencv2\opencv.cpp>
#include "mex.h"
#include <string>
#include <iostream>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace cv;

// input: test samples NxD
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs != 1 || nlhs != 1)
        mexErrMsgTxt("Wrong number of parameters.");

    // input
    int samprow = mxGetM(prhs[0]);
    int sampcol = mxGetN(prhs[0]);
    
    double *pSamp = mxGetPr(prhs[0]);
    
    // convert to opencv format
    Mat samps(samprow, sampcol, CV_32F);
    for(int r=0; r<samprow; r++)
        for(int c=0; c<sampcol; c++)
            samps.at<float>(r,c) = pSamp[r*sampcol+c];

    
    // do classification for each sample
    // output
    plhs[0] = mxCreateDoubleMatrix(samprow, 1, mxREAL);
    double *pOut = mxGetPr(plhs[0]);
    vector<double> scores;
    
    
    return;
}

