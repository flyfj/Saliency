/**********************************************************************************************/
/* Calculates a probabilistic boundary edge map using Color and texture information in an image*/
/* Author: Ajay Mishra (mishraka@gmail.com)     					       */
/* Date  : Oct 20, 2009									       */
/* Copyright 2009 University of Maryland at College Park. All rights reserved.                 */
/***********************************************************************************************/   

#include <opencv2/opencv.hpp>
#include "savgol.h"
#include "textureGradient.h"

//------------------------------------------------------------------------------
//   Color and Gradient based edge detector
//------------------------------------------------------------------------------
void pbCGTG(IplImage *im, IplImage* grad, IplImage* ori){
	double BETA[] = {-4.5016,    1.6921,    0.9562,    1.0045,    2.8116};

	//1. Calculate color and texture gradient gradient	
    	printf("\nCalculating Color gradient");
	int norient = 8;
	double *gtheta =(double *) malloc (sizeof(double)*norient);
	CvMat** cg    = detCG(im, norient, gtheta);

 
	printf("\nCalculating texture gradient");
	CvMat** tg     = detTG(im, norient, gtheta);
   	printf("\n");

	//2. Compute oriented Pb
	CvMat* a;
	CvMat* b;
	CvMat* l;
	CvMat* t;
	CvMat** orientedPb = new CvMat* [norient];
	for(int i=0; i< norient; i++){
		orientedPb[i] = cvCreateMat(im->height, im->width, CV_32FC1);
		a = cg[1*norient+i];
		b = cg[2*norient+i];
		l = cg[i];
		t = tg[i];
		for(int h=0; h < im->height; h++){
			for(int w=0; w < im->width; w++){
				double a_grad = cvGetReal2D(a,h,w);
				double b_grad = cvGetReal2D(b,h,w);
				double l_grad = cvGetReal2D(l,h,w);
				double t_grad = cvGetReal2D(t,h,w);
				double tmpSum = BETA[0] + l_grad*BETA[1]+ a_grad*BETA[2] + b_grad*BETA[3] + t_grad*BETA[4]; 
				cvSetReal2D(orientedPb[i], h, w, 1/(1+exp(-tmpSum)) );
			}
		}
		//Release Matrices
		cvReleaseMat(&a);
		cvReleaseMat(&b);
		cvReleaseMat(&l);
		cvReleaseMat(&t);
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

//------------------------------------------------------------------------------
//   Color and Gradient based edge detector (without maximal supression)
//------------------------------------------------------------------------------
void pbCGTG_NoMaxSup(IplImage *im, IplImage* grad, IplImage* ori){
	double BETA[] = {-4.5016,    1.6921,    0.9562,    1.0045,    2.8116};

	//1. Calculate color and texture gradient gradient	
    	printf("\nCalculating Color gradient");
	int norient = 8;
	double *gtheta =(double *) malloc (sizeof(double)*norient);
	CvMat** cg     = detCG(im, norient, gtheta);

 
	printf("\nCalculating texture gradient");
	CvMat** tg     = detTG(im, norient, gtheta);
   	printf("\n");
	
	//2. Compute oriented Pb
	CvMat* a;
	CvMat* b;
	CvMat* l;
	CvMat* t;
	CvMat** orientedPb = new CvMat* [norient];
	for(int i=0; i< norient; i++){
		orientedPb[i] = cvCreateMat(im->height, im->width, CV_32FC1);
		a = cg[1*norient+i];
		b = cg[2*norient+i];
		l = cg[i];
		t = tg[i];
		for(int h=0; h < im->height; h++){
			for(int w=0; w < im->width; w++){
				double a_grad = cvGetReal2D(a,h,w);
				double b_grad = cvGetReal2D(b,h,w);
				double l_grad = cvGetReal2D(l,h,w);
				double t_grad = cvGetReal2D(t,h,w);
				double tmpSum = BETA[0] + l_grad*BETA[1]+ a_grad*BETA[2] + b_grad*BETA[3] + t_grad*BETA[4]; 
				cvSetReal2D(orientedPb[i], h, w, 1/(1+exp(-tmpSum)) );
			}
		}
		//Release Matrices
		cvReleaseMat(&a);
		cvReleaseMat(&b);
		cvReleaseMat(&l);
		cvReleaseMat(&t);
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
		pbi = fitparab(*(orientedPb[i]),r,r,gtheta[i]);
		//pbi = nonmaxsup(tmp,gtheta[i]);
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



//------------------------------------------------------------------------------
// Non-maximal suppression
//------------------------------------------------------------------------------
CvMat* nonmaxsup(CvMat* prob, double theta){

	double theta_orthog = theta+M_PI/2;
	theta_orthog  = fmod(theta_orthog,M_PI);
	double d;

	CvMat* probThinned = cvCreateMat(prob->rows,prob->cols,CV_32FC1);
	cvCopy(prob,probThinned);
	CvMat* edgeMask = cvCreateMat(prob->rows,prob->cols,CV_32FC1);
	cvSetZero(edgeMask);
	cvSubRS(edgeMask,cvScalarAll(1.0),edgeMask);
	CvScalar a_value;
	CvScalar b_value;
	CvScalar pixValue;
	if(theta_orthog < M_PI/4 && theta_orthog >= 0){
		d  = tan(theta_orthog);
		double neighboringValue = 0;
		for(int r=0; r < prob->rows; r++){
			for(int c=0; c < prob->cols; c++){
				// case 1
				if(r < prob->rows-1 && c < prob->cols-1){
					a_value = cvGet2D(prob,r,c+1);
					b_value = cvGet2D(prob,r+1,c+1);
					neighboringValue = a_value.val[0]*(1-d)+b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
				// case 5
				if(r > 0 && c > 0){
					a_value = cvGet2D(prob,r,c-1);
					b_value = cvGet2D(prob,r-1,c-1);
					neighboringValue = a_value.val[0]*(1-d)+b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
			}
		}
	}
	else if(theta_orthog < M_PI/2 && theta_orthog >= M_PI/4){
		d  = tan(M_PI/2-theta_orthog);
		double neighboringValue = 0;
		for(int r=0; r < prob->rows; r++){
			for(int c=0; c < prob->cols; c++){
				// case 2
				if(r < prob->rows-1 && c < prob->cols-1){
					a_value = cvGet2D(prob,r+1,c);
					b_value = cvGet2D(prob,r+1,c+1);
					neighboringValue = a_value.val[0]*(1-d)+ b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
				// case 6
				if(r > 0 && c > 0){
					a_value = cvGet2D(prob,r-1,c);
					b_value = cvGet2D(prob,r-1,c-1);
					neighboringValue = a_value.val[0]*(1-d)+ b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
			}
		}

	}
	else if(theta_orthog < 3*M_PI/4 && theta_orthog >= M_PI/2){
		d = tan(theta_orthog - M_PI/2);
		double neighboringValue = 0;
		for(int r=0; r < prob->rows; r++){
			for(int c=0; c < prob->cols; c++){
				// case 3
				if(r < prob->rows-1 && c > 0){
					a_value = cvGet2D(prob,r+1,c);
					b_value = cvGet2D(prob,r+1,c-1);
					neighboringValue = a_value.val[0]*(1-d) + b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
				// case 7
				if(r > 0 && c < prob->cols-1){
					a_value = cvGet2D(prob,r-1,c);
					b_value = cvGet2D(prob,r-1,c+1);
					neighboringValue = a_value.val[0]*(1-d) + b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
			}
		}
	}
	else if(theta_orthog < M_PI && theta_orthog >= 3*M_PI/4){
		d  = tan(M_PI - theta_orthog);
		double neighboringValue = 0;
		for(int r=0; r < prob->rows; r++){
			for(int c=0; c < prob->cols; c++){
				// case 4
				if(r < prob->rows-1 && c > 0){
					a_value = cvGet2D(prob,r,c-1);
					b_value = cvGet2D(prob,r+1,c-1);
					neighboringValue = a_value.val[0]*(1-d) + b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
				// case 8
				if(r > 0 && c < prob->cols-1){
					a_value = cvGet2D(prob,r,c+1);
					b_value = cvGet2D(prob,r-1,c+1);
					neighboringValue = a_value.val[0]*(1-d) + b_value.val[0]*d;
					pixValue = cvGet2D(prob,r,c);
					if (neighboringValue > pixValue.val[0])
						cvSet2D(edgeMask,r,c,cvScalar(0.0));
				}
			}
		}
	}

	// multiply the probibility values with the mask
	cvMul(prob,edgeMask,probThinned);

	cvReleaseMat(&edgeMask);


	return probThinned;
}


/*
//---------------
// MAIN FUNCTION
//--------------
int main(int argc, char *argv[]){
    
	// check the arguments!
	if (argc < 2 ){
		fprintf(stderr,"needs at least one argument: <color image name>");
		exit(1);
	}
	
	char* inputImgName = argv[1];
	IplImage* inputImg_uchar = cvLoadImage(inputImgName);
	if( inputImg_uchar == NULL){
		fprintf(stderr,"\n unable to read %s",inputImgName);
		exit(1);
    	}

	//Show the original image
	cvNamedWindow("Original image"); cvShowImage("Original image",inputImg_uchar);
	cvWaitKey(-1);

	IplImage *img_float = cvCreateImage(cvGetSize(inputImg_uchar),IPL_DEPTH_32F,inputImg_uchar->nChannels);
	cvConvertScale(inputImg_uchar, img_float,1.0);


	//Output images
	IplImage* grad_float = cvCreateImage(cvGetSize(img_float),IPL_DEPTH_32F,1);	cvSetZero(grad_float);
	IplImage* ori_float  = cvCreateImage(cvGetSize(img_float),IPL_DEPTH_32F,1);	cvSetZero(ori_float);

	// the color gradient based edge detector!!
	pbCGTG(img_float,grad_float,ori_float);

	// Show the gradient image 
	cvNamedWindow("gradient image"); cvShowImage("gradient image",grad_float);
	cvWaitKey(-1);
	
	//release images	
	cvReleaseImage(&inputImg_uchar);	
	cvReleaseImage(&img_float);	
	cvReleaseImage(&grad_float);	
	cvReleaseImage(&ori_float);
	return 0;
  }
*/
