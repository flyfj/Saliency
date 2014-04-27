#include "mex.h"
#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <string>

#ifdef _WIN32
#include "ppl.h"
using namespace concurrency;
#else
#include "omp.h"
#endif

double dist(const double *p1, const double *p2) {
    return sqrt((p1[0] - p2[0]) * (p1[0] - p2[0]) + 
                (p1[1] - p2[1]) * (p1[1] - p2[1]) + 
                (p1[2] - p2[2]) * (p1[2] - p2[2]));
}

struct distPair {
    int id;
    float dist;
    distPair(): id(0), dist(0) {}
    distPair(int i, float d): id(i), dist(d) {}
};

using namespace std;

// cellarr = ComputeNeighbors(v, k)
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs != 2 || nlhs > 1)
		mexErrMsgTxt("Wrong number of parameters. cellarr = ComputeNeighbors(v, k);");

    // parse the input
    #define V_IN prhs[0]
    #define K_IN prhs[1]
    #define N_OUT plhs[0]
    if (mxGetM(V_IN) != 3)
        mexErrMsgTxt("v must have 3 rows.");
    int vn = mxGetN(V_IN);
    int k = mxGetScalar(K_IN);

    // allocate the output memory
    vector<int> dim;
    dim.push_back(vn);
    N_OUT = mxCreateCellArray(1, &dim[0]);

    // compute neighbors
	const double *pv = mxGetPr(V_IN);
    #ifdef _WIN32
    critical_section cs;
    parallel_for(0, vn, [&](int i) {
    #else
    #pragma omp parallel for
    for (int i = 0; i < vn; i++) {
    #endif
        vector<distPair> dists(vn);
        for (int j = 0; j < vn; j++) {
            dists[j] = distPair(j + 1, dist(pv + i * 3, pv + j * 3));
        }
        sort(dists.begin(), dists.end(), [](const distPair& a, const distPair &b) { return a.dist < b.dist; });
        
        // write the result back 
        #ifdef _WIN32
        cs.lock();
        #else
        #pragma omp critical
        {
        #endif
        size_t kEnd = vn < k ? vn : k;
        mxArray *nns = mxCreateDoubleMatrix(1, kEnd, mxREAL);
        double *pNns = mxGetPr(nns);
        for (int kk = 0; kk < kEnd; kk++) {
            pNns[kk] = dists[kk].id;
        }
        mxSetCell(N_OUT, i, nns);
        #ifdef _WIN32
        cs.unlock();
        #else
        }
        #endif

        if (!(i % 5000)) {
            mexPrintf("%d/%d\n", i, vn);
        }
    #ifdef _WIN32
    });
    #else
    }
    #endif

	return;
}
