//Object boundary detection using local processing 
//Author: Ajay K Mishra
//Date: July 19, 2010
//Modified on: Dec 01, 2010
//-------------------------------------------------
#include<stdio.h>
#include<cmath>
#include"pbBoundary.h"
#include <opencv2/opencv.hpp>
#ifndef M_PI
#define M_PI acos(-1.0)
#endif


// f: R -> [0,1]
inline double sigmoid(	double x, 
					double beta1, 
					double beta2)
{
  return 1.0/(1.0+exp(-beta1*(x - beta2)));
}


// Create oriented circular half discs
void createDiscFilters(	int numOri, 
					int wr,
					int gap, 
					IplImage** leftDisc, 
					IplImage** rightDisc)
{
  
	for(int i=0; i<numOri; i++){
    		leftDisc[i]		= cvCreateImage(cvSize(2*wr+1,2*wr+1),IPL_DEPTH_32F,1);
    		rightDisc[i]		= cvCreateImage(cvSize(2*wr+1,2*wr+1),IPL_DEPTH_32F,1);    
    		double leftDiscTot =0;
    		double rightDiscTot = 0;
    		double binSize = M_PI/numOri;
    		for(int y=-wr; y<=wr; y++){
      		for(int x=-wr; x <=+wr; x++){
				double radius = sqrt((double)(x*x+y*y));
				if ((radius > wr) || ((x == 0) && (y == 0)) ){
	  				CV_IMAGE_ELEM(leftDisc[i],float,y+wr,x+wr) = 0;
	  				CV_IMAGE_ELEM(rightDisc[i],float,y+wr,x+wr) = 0;
				}
				else{
	  				double theta = atan2((double)y,(double)x)-(i*binSize); // to orient the disc!
	  				theta = theta < 0? 2*M_PI+theta: theta;	  
	  				double proj = fabs(x*cos(i*binSize+M_PI/2) + y*sin(i*binSize+M_PI/2));
	  				double value = (proj < gap) ? 0.01: 1;

	  				CV_IMAGE_ELEM(leftDisc[i],float,y+wr,x+wr) = (theta < M_PI)   ? value :0;
	  				CV_IMAGE_ELEM(rightDisc[i],float,y+wr,x+wr)= (theta >= M_PI)  ? value :0;	
	  				if (theta < M_PI){ 
	  					leftDiscTot += value;
	  				}
	  				else{
	  					rightDiscTot += value;
	  				}		  
				}	
      		}
    		}          
    		// normalize
    		int count1 = cvCountNonZero(leftDisc[i]);
    		int count2 = cvCountNonZero(rightDisc[i]);
    		if (count1 != count2){
      		printf("Discs imBalance: count1 %d count2 %d\n",count1,count2);
      		exit(1);
    		}    
    		cvConvertScale(leftDisc[i],	leftDisc[i],	1.0/leftDiscTot);
    		cvConvertScale(rightDisc[i],	rightDisc[i],	     1.0/rightDiscTot);
  	}
}


// calculate c1c2c3 based 3D color histogram
void getColorHist(IplImage** imgc1c2c3_float, 
				const int numImg, 
				double* hist_c1c2c3, 
				const int nbins_L, 
				const int nbins_a, 
				const int nbins_b)
{
	// initialize	
  	for(int i=0; i<nbins_L*nbins_a*nbins_b; i++){
    		hist_c1c2c3[i] =0; 
  	}
  	int numPix = 0;
  	for(int n=0; n<numImg; n++){
    		for(int x=0; x < imgc1c2c3_float[n]->width; x++){
      		for(int y=0; y < imgc1c2c3_float[n]->height; y++){
				int binInd_L = floor(nbins_L*CV_IMAGE_ELEM(imgc1c2c3_float[n], float, y, 3*x)*1.9999/M_PI);
				int binInd_a = floor(nbins_a*CV_IMAGE_ELEM(imgc1c2c3_float[n], float, y, 3*x+1)*1.9999/M_PI);
				int binInd_b = floor(nbins_b*CV_IMAGE_ELEM(imgc1c2c3_float[n], float, y, 3*x+2)*1.9999/M_PI); 		
				hist_c1c2c3[nbins_L*nbins_a*binInd_b + nbins_L*binInd_a + binInd_L]++;
      		}
    		}
    		numPix += imgc1c2c3_float[n]->width*imgc1c2c3_float[n]->height;
  	}  
  	//normalize the histogram
  	for(int i=0; i<nbins_L*nbins_a*nbins_b; i++){
    		hist_c1c2c3[i] /= numPix; 
  	}
}



void calcHistOfDisc(IplImage* imgc1c2c3_float, 
				CvRect bdBox, 
				IplImage* leftMask_float, 
				IplImage* rightMask_float, 
				double* leftHist, 
				double* rightHist, 
				int nL, int na, int nb)
{
	for(int i=0; i<nL*na*nb; i++){
    		leftHist[i]  = 0;
    		rightHist[i] = 0;
  	}

  	for(int x=bdBox.x; x < (bdBox.x + bdBox.width); x++){
    		for(int y=bdBox.y; y < (bdBox.y + bdBox.height); y++){
      		if(CV_IMAGE_ELEM(leftMask_float, float, y-bdBox.y, x-bdBox.x) > 0){
				int binInd_L = floor(nL*CV_IMAGE_ELEM(imgc1c2c3_float, float, y, 3*x)*1.9999/M_PI);
				int binInd_a = floor(na*CV_IMAGE_ELEM(imgc1c2c3_float, float, y, 3*x+1)*1.9999/M_PI);
				int binInd_b = floor(nb*CV_IMAGE_ELEM(imgc1c2c3_float, float, y, 3*x+2)*1.9999/M_PI);
				leftHist[nL*na*binInd_b + nL*binInd_a + binInd_L] += CV_IMAGE_ELEM(leftMask_float, float,y-bdBox.y, x-bdBox.x );
      		}
      		if(CV_IMAGE_ELEM(rightMask_float, float, y-bdBox.y, x-bdBox.x) > 0){
				int binInd_L = floor(nL*CV_IMAGE_ELEM(imgc1c2c3_float, float, y, 3*x)*1.9999/M_PI);
				int binInd_a = floor(na*CV_IMAGE_ELEM(imgc1c2c3_float, float, y, 3*x+1)*1.9999/M_PI);
				int binInd_b = floor(nb*CV_IMAGE_ELEM(imgc1c2c3_float, float, y, 3*x+2)*1.9999/M_PI);
				rightHist[nL*na*binInd_b + nL*binInd_a + binInd_L] += CV_IMAGE_ELEM(rightMask_float, float,y-bdBox.y, x-bdBox.x);
      		}
    		}
  	}    
} 


//CHI-2 distance between histograms
inline double 
chiSquareDist(	double* h1, 
			double* h2, 
			const int len)
{
	double dist=0;
  	for(int i=0; i<len; i++){
    		if ((h1[i]+h2[i])>0){
      		dist += ((h1[i]-h2[i])*(h1[i]-h2[i]))/(h1[i]+h2[i]);
    		}
  	}
  return (dist/2);
}


//********************************************************************************
// BIASING the edge map using different cues 
//********************************************************************************

//With surface distribution
void calcPbBoundaryWtSurface( IplImage* imgLab_uchar, 
						IplImage* edgeMap_float, 
						IplImage* edgeOri_float, 
						IplImage* bdOwnership_float, 
						double* surfaceHist, 
						const int numOri, 
						const int nL, 
						const int na, 
						const int nb)
{
	//Initialize the parameters of the logistic function!
	double beta1 = 8;   
	double beta2 = 0.3;

  	int w = 0.01*sqrt((double)(edgeMap_float->width*edgeMap_float->width + edgeMap_float->height*edgeMap_float->height));
  
  	// Create half-disc filters!!
  	IplImage** leftDisc;
  	IplImage** rightDisc;
  	leftDisc	= (IplImage**)malloc(numOri*sizeof(IplImage*));	
  	rightDisc	= (IplImage**)malloc(numOri*sizeof(IplImage*)); 
	int gap = 0; 
  	createDiscFilters(numOri, w, gap, leftDisc, rightDisc);
   
  	// histograms!
  	double* leftHist  = (double*) malloc(sizeof(double)*nL*na*nb);
  	double* rightHist = (double*) malloc(sizeof(double)*nL*na*nb);


  	// convolve the map with the filters..!!
  	float oriBinSize        = M_PI/numOri;				
  	for(int y = w; y < edgeMap_float->height-w-1; y++){
    		for( int x = w; x < edgeMap_float->width-w-1; x++){
      		if(CV_IMAGE_ELEM(edgeMap_float,float,y,x) > 0){
				int oriInd = floor(fmod(CV_IMAGE_ELEM(edgeOri_float,float,y,x)/oriBinSize,numOri));	
	
				//calculate histogram
				calcHistOfDisc(imgLab_uchar,
							cvRect(x-w,y-w,2*w+1,2*w+1),
							leftDisc[oriInd], 
							rightDisc[oriInd], 
							leftHist, rightHist, 
							nL, na, nb);
	
				//Use X2 distribution instead!
				double d_left  = chiSquareDist(surfaceHist, leftHist, nL*na*nb);
				double d_right = chiSquareDist(surfaceHist, rightHist, nL*na*nb);
	
				double leftProb  = 1-sigmoid(d_left,  beta1, beta2);
				double rightProb = 1-sigmoid(d_right, beta1, beta2);
			
				CV_IMAGE_ELEM(edgeMap_float, float, y, x) = fabs(leftProb-rightProb);
				if ( (leftProb - rightProb) > 0.5){
					CV_IMAGE_ELEM(bdOwnership_float, float, y, x) = -1;
				}
				else if( (rightProb - leftProb) > 0.5){
					CV_IMAGE_ELEM(bdOwnership_float, float, y, x) = +1;
				}
      		}
    		}
  	}
	
	//Release!!
  	for(int i=0; i<numOri; i++){
    		cvReleaseImage(&leftDisc[i]);
    		cvReleaseImage(&rightDisc[i]);
  	}
  	free(leftDisc);
  	free(rightDisc);
  	free(leftHist);
  	free(rightHist);
}

//With disparity map
void calcPbBoundaryWtDisparity(	IplImage* edgeMap, 
							IplImage* edgeOri, 
							IplImage* disparity, 
							int numOri)
{
	//Initialize the parameters of the logistic function!
	double beta1 = 10;   
	double beta2 = 0.3;
	int w = 0.015*sqrt((double)(edgeMap->width*edgeMap->width + edgeMap->height*edgeMap->height));
  
	// Create half-disc filters!!
  	IplImage** leftDisc;
  	IplImage** rightDisc;
  	leftDisc	= new IplImage*[numOri];
  	rightDisc	= new IplImage*[numOri];
	int gap = w/2.5;
  	createDiscFilters(numOri, w, gap, leftDisc, rightDisc);
   
  	// convolve the map with the filters..!!
  	float oriBinSize = M_PI/numOri;
  	IplImage* multResult = cvCreateImage(cvSize(2*w+1,2*w+1),IPL_DEPTH_32F,1);
  	CvScalar leftSum;
  	CvScalar rightSum;				
  	for(int y = w; y < edgeMap->height-w; y++){
    		for( int x = w; x < edgeMap->width-w; x++){
      		if(CV_IMAGE_ELEM(edgeMap,float,y,x) > 0){
				int oriInd = floor(fmod(CV_IMAGE_ELEM(edgeOri,float,y,x)/oriBinSize,numOri));
				cvSetImageROI(disparity,cvRect(x-w,y-w,2*w+1,2*w+1));
				//left
				cvMul(disparity,leftDisc[oriInd],multResult);
				leftSum = cvSum(multResult);	
				//right
				cvMul(disparity,rightDisc[oriInd],multResult);
				rightSum = cvSum(multResult);					
				double diffDisp = fabs(leftSum.val[0]-rightSum.val[0]);	
				double prob     = sigmoid(diffDisp, beta1, beta2);	
				CV_IMAGE_ELEM(edgeMap,float,y,x) = (prob > 50.0/255)? prob: 50.0/255;
      		}
    		}
  	}
  
  	//release
  	for(int i=0; i<numOri; i++){
    		cvReleaseImage(&leftDisc[i]);
    		cvReleaseImage(&rightDisc[i]);
  	}
  	delete [] leftDisc;
  	delete [] rightDisc;  
  	cvReleaseImage(&multResult);
}


//Using probabilistic object Mask (0< val < 1)
void calcPbBoundaryWtObjMask( IplImage* edgeMap,
						IplImage* edgeOri,
						IplImage* objMask,
						IplImage* bdOwnership_float,
						int numOri) 
{    
	//Initialize the parameters of the logistic function!
	double beta1 = 10;   
	double beta2 = 0.1;	
	// Create half-disc filters!!
	int w = 0.020*sqrt((double)(edgeMap->width*edgeMap->width + edgeMap->height*edgeMap->height));  
  	IplImage** leftDisc;
  	IplImage** rightDisc;
  	leftDisc	= new IplImage*[numOri];
  	rightDisc	= new IplImage*[numOri];
	int gap = w/3.0;
  	createDiscFilters(numOri, w, gap, leftDisc, rightDisc);
  
	IplImage* objMask_padded = cvCreateImage(cvSize(2*w+objMask->width+1, 2*w+objMask->height+1), IPL_DEPTH_32F,1);
 	cvCopyMakeBorder(objMask, objMask_padded, cvPoint(w,w), IPL_BORDER_REPLICATE);

	float oriBinSize = M_PI/numOri;
  	IplImage* multResult = cvCreateImage(cvSize(2*w+1,2*w+1),IPL_DEPTH_32F,1);
  	CvScalar leftSum;
  	CvScalar rightSum;				
  	for(int y = 0; y < edgeMap->height; y++){
    		for( int x = 0; x < edgeMap->width; x++){
      		if(CV_IMAGE_ELEM(edgeMap,float,y,x) > 0){
				int oriInd = floor(fmod(CV_IMAGE_ELEM(edgeOri,float,y,x)/oriBinSize,numOri));
				cvSetImageROI(objMask_padded,cvRect(x,y,2*w+1,2*w+1));
				
				//left & right sums
				cvMul(objMask_padded,leftDisc[oriInd],multResult);
				leftSum 	= cvSum(multResult);
				cvMul(objMask_padded,rightDisc[oriInd],multResult);
				rightSum 	= cvSum(multResult);

				//convert into pb
				float prob = sigmoid(fabs(leftSum.val[0]-rightSum.val[0]), beta1, beta2);
				CV_IMAGE_ELEM(edgeMap,float,y,x) = (prob > 50.0/255)? prob: 50.0/255;

				//assign  border ownership!	
				CV_IMAGE_ELEM(bdOwnership_float, float, y, x) = (leftSum.val[0] > rightSum.val[0]) ? +1: -1;			
			}
		}
	}	

  	//release memory!
  	for(int i=0; i<numOri; i++){
    		cvReleaseImage(&leftDisc[i]);
    		cvReleaseImage(&rightDisc[i]);
  	}
  	delete [] leftDisc;
  	delete [] rightDisc;
  
  	cvReleaseImage(&multResult);
  	cvReleaseImage(&objMask_padded);
}

//Using optic flow map (u,v)
void calcPbBoundaryWtFlow(	IplImage* edgeMap, 
						IplImage* edgeOri,
						IplImage* bdOwnership_float, 
						IplImage* flow_u, 
						IplImage* flow_v, 
						int numOri)
{  
	//Initialize the parameters of the logistic function!
	double beta1 = 10;   
	double beta2 = 0.6;
	//Create half-disc filters!!
	int w = 0.020*sqrt((double)(edgeMap->width*edgeMap->width + edgeMap->height*edgeMap->height));    	
  	IplImage** leftDisc;
  	IplImage** rightDisc;
  	leftDisc	= new IplImage*[numOri];
  	rightDisc	= new IplImage*[numOri];
	int gap = w/2.5;
  	createDiscFilters(numOri, w, gap, leftDisc, rightDisc);
 
  	IplImage* flow_u_padded = cvCreateImage(cvSize(2*w+flow_u->width+1, 2*w+flow_u->height+1), IPL_DEPTH_32F,1);
  	IplImage* flow_v_padded = cvCreateImage(cvSize(2*w+flow_v->width+1, 2*w+flow_v->height+1), IPL_DEPTH_32F,1);
  	cvCopyMakeBorder(flow_u, flow_u_padded, cvPoint(w,w), IPL_BORDER_REPLICATE);
  	cvCopyMakeBorder(flow_v, flow_v_padded, cvPoint(w,w), IPL_BORDER_REPLICATE);
 
  	// convolve the map with the filters..!!
  	float oriBinSize = M_PI/numOri;
  	IplImage* multResult = cvCreateImage(cvSize(2*w+1,2*w+1),IPL_DEPTH_32F,1);
  	CvScalar leftSum_u, leftSum_v;
  	CvScalar rightSum_u, rightSum_v;				
  	for(int y = 0; y < edgeMap->height; y++){
    		for( int x = 0; x < edgeMap->width; x++){
      		if(CV_IMAGE_ELEM(edgeMap,float,y,x) > 0){
				int oriInd = floor(fmod(CV_IMAGE_ELEM(edgeOri,float,y,x)/oriBinSize,numOri));
				cvSetImageROI(flow_u_padded,cvRect(x,y,2*w+1,2*w+1));
				cvSetImageROI(flow_v_padded,cvRect(x,y,2*w+1,2*w+1));	
				//left
				cvMul(flow_u_padded,leftDisc[oriInd],multResult);
				leftSum_u = cvSum(multResult);
				cvMul(flow_v_padded,leftDisc[oriInd],multResult);
				leftSum_v = cvSum(multResult);	
				//right
				cvMul(flow_u_padded,rightDisc[oriInd],multResult);
				rightSum_u = cvSum(multResult);				
				cvMul(flow_v_padded,rightDisc[oriInd],multResult);
				rightSum_v = cvSum(multResult);					
				double vecDiff = 	(leftSum_u.val[0]-rightSum_u.val[0])*(leftSum_u.val[0]-rightSum_u.val[0]) + \
	  							(leftSum_v.val[0]-rightSum_v.val[0])*(leftSum_v.val[0]-rightSum_v.val[0]) ;
				vecDiff        = sqrt(vecDiff);	
				float prob = sigmoid(vecDiff, beta1, beta2);
				CV_IMAGE_ELEM(edgeMap,float,y,x) = (prob > 50.0/255)? prob: 50.0/255;

				//decides border ownership!
				double leftMag          = sqrt(leftSum_u.val[0]* leftSum_u.val[0] + leftSum_v.val[0]* leftSum_v.val[0]); 	
				double rightMag         = sqrt(rightSum_u.val[0]* rightSum_u.val[0] + rightSum_v.val[0]* rightSum_v.val[0]);
	
				if ( leftMag > rightMag && prob > 0.5){
					CV_IMAGE_ELEM(bdOwnership_float, float, y, x) = +1;
				}
				else if( leftMag < rightMag && prob > 0.5){
					CV_IMAGE_ELEM(bdOwnership_float, float, y, x) = -1;
				} 		
      		}
    		}
  	} 
  	//release
  	for(int i=0; i<numOri; i++){
    		cvReleaseImage(&leftDisc[i]);
    		cvReleaseImage(&rightDisc[i]);
  	}
  	delete [] leftDisc;
  	delete [] rightDisc;  
  	cvReleaseImage(&multResult);
  	cvReleaseImage(&flow_u_padded);
  	cvReleaseImage(&flow_v_padded);
}
