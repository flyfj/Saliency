/***********************************************************************************************/
/* Calculates a probabilistic boundary edge map using brightness information in the image      */
/* Author: Ajay Mishra (mishraka@gmail.com)     					       */
/* Date  : Oct 20, 2009									       */
/* Copyright 2009 University of Maryland at College Park. All rights reserved.                 */
/***********************************************************************************************/   

#include <opencv2/opencv.hpp>
#include "savgol.h"

//------------------------------------------------------------------------------
//   Color and Gradient based edge detector
//------------------------------------------------------------------------------
void pbBG(IplImage *im, IplImage* grad, IplImage* ori){
	double BETA[] = {-3.6945, 2.7430};

	//1. Calculate color and texture gradient gradient
	int norient = 8;
	double *gtheta =(double *) malloc (sizeof(double)*norient);
        fprintf(stdout,"\nCalculating Brightness gradient"); fflush(stdout);
	CvMat** bg     = detBG(im, norient, gtheta);
	
	//2. Compute oriented Pb
	CvMat* b;
	CvMat** orientedPb = new CvMat* [norient];
	for(int i=0; i< norient; i++){
		orientedPb[i] = cvCreateMat(im->height, im->width, CV_32FC1);
		b = bg[i];
		for(int h=0; h < im->height; h++){
			for(int w=0; w < im->width; w++){
				double b_grad = cvGetReal2D(b,h,w);
				double tmpSum = BETA[0] + b_grad*BETA[1]; 
				cvSetReal2D(orientedPb[i], h, w, 1/(1+exp(-tmpSum)) );
			}
		}
		//Release Matrices
		cvReleaseMat(&b);
	}

	//3. nonmax suppression and max over orientations
	CvMat* maxOri   = cvCreateMat(im->height,im->width,CV_32FC1);
	for(int h=0; h < im->height; h++){
		for(int w=0; w < im->width; w++){
			int maxOriInd=0;
			for(int oriInd=0; oriInd < norient; oriInd++){
				if ( cvGetReal2D(orientedPb[maxOriInd],h,w) < cvGetReal2D(orientedPb[oriInd],h,w) )
					maxOriInd = oriInd;
			}
			cvSetReal2D(maxOri,h,w,(float)maxOriInd);
		}
	}
	CvMat* pb     = cvCreateMat(im->height, im->width, CV_32FC1); cvSetZero(pb);	
	CvMat* pbi    = cvCreateMat(im->height, im->width, CV_32FC1); cvSetZero(pb);
	CvMat* theta  = cvCreateMat(im->height, im->width, CV_32FC1); cvSetZero(theta);
	double r =2.5;
	CvMat* tmp; // = cvCreateMat(im->height, im->width, CV_32FC1);
	for(int i=0; i < norient; i++){
		tmp = fitparab(*(orientedPb[i]),r,r,gtheta[i]);
		pbi = nonmaxsup(tmp,gtheta[i]);
		for(int h=0; h < pb->rows; h++){
			for(int w=0; w < pb->cols; w++){
				if( cvGet2D(maxOri,h,w).val[0] == i && cvGet2D(pb,h,w).val[0] < cvGet2D(pbi,h,w).val[0]){
					cvSetReal2D(pb,h,w,cvGetReal2D(pbi,h,w));
				}
				if (cvGet2D(maxOri,h,w).val[0] == i){
					cvSetReal2D(theta,h,w,gtheta[i]);
				}
			}	
		}
	}


	// "pb" should be between 0 and 1 
	for(int h=0; h < pb->rows; h++){
		for( int w=0; w< pb->cols; w++){
			if(cvGet2D(pb,h,w).val[0] < 0)
				cvSet2D(pb,h,w,cvScalar(0.0));
			if(cvGet2D(pb,h,w).val[0] > 1)
				cvSet2D(pb,h,w,cvScalar(1.0));
		}
	}


	// Copy cvMat into IplImages
	for(int i=0; i<im->height; i++){
		for(int j=0; j<im->width; j++){
			cvSetReal2D(grad,i,j,cvGetReal2D(pb,i,j));
			cvSetReal2D(ori,i,j,cvGetReal2D(theta,i,j));
		}
	}


	//release matrices!!
	for(int i=0; i<norient; i++)
		cvReleaseMat(&orientedPb[i]);
	
	cvReleaseMat(&pb);
	cvReleaseMat(&pbi);
	cvReleaseMat(&theta);
	cvReleaseMat(&maxOri);
	free(gtheta);

	return;

}
