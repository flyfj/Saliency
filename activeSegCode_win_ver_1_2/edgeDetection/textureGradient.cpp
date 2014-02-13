#include "savgol.h"
#include "textureGradient.h"
//#include "unistd.h"
#include <windows.h>
#include "stdio.h"

char filterBin_dirLoc[1000]; 

#ifndef MIN
#define MIN(x,y) (x < y)? x: y
#endif

void findWorkDir(void){
	wchar_t lpBuffer[MAX_PATH];
	int bytes = GetModuleFileName( NULL, (LPTSTR)lpBuffer, MAX_PATH);
	for(int i=2*bytes-1; i>0; i=i-1){
		if(lpBuffer[i]=='\\'){
			lpBuffer[i] = '\0';
			bytes = i;
			break;
		}
	}
	for(int i=0; i<=bytes; i++)
		filterBin_dirLoc[i]= lpBuffer[i];	
	//printf("\n after token stuff.. %s \n", filterBin_dirLoc);
} 

typedef struct {
	CvMat* filterBank[24];
	CvMat* textons;
}textonData;

void assignTextons(CvMat* tmap, CvMat* grayImg){
	//char filterBin_dirLoc[1000];
	findWorkDir();
	// Run the filter bank on grey image!!
	// 1. Read the filter Bank parameters
	// 2. convolve the image with the filter to get the response.
	char filterBankFileName[1000];
	strcpy(filterBankFileName, filterBin_dirLoc);
	strcat(filterBankFileName,"/texture_filterbank_13.txt");
	FILE* fp_13 = fopen(filterBankFileName,"r");

	strcpy(filterBankFileName, filterBin_dirLoc);
	strcat(filterBankFileName,"/texture_filterbank_19.txt");
	FILE* fp_19 = fopen(filterBankFileName,"r");

	if (fp_13 == NULL || fp_19 == NULL){
		fprintf(stderr,"either texture_filterbank_19.txt or texture_filterbank_13.txt or both not found\n");
		exit(1);
	}
	//double*    tmp1= (double*)malloc(13*13*sizeof(double));
	//double*    tmp2= (double*)malloc(19*19*sizeof(double));
	CvMat*     filterBank13x13 = cvCreateMat(13,13,CV_32FC1);
	CvMat*     filterBank19x19 = cvCreateMat(19,19,CV_32FC1);

	CvMat* imgResponse[24];
	CvMat* grayImg_pad13x13     = cvCreateMat(grayImg->rows+6,grayImg->cols+6,CV_32FC1);
	CvMat* grayImg_pad19x19     = cvCreateMat(grayImg->rows+9,grayImg->cols+9,CV_32FC1);
	CvMat* imgResponse_pad13x13 = cvCreateMat(grayImg->rows+6,grayImg->cols+6,CV_32FC1);
	CvMat* imgResponse_pad19x19 = cvCreateMat(grayImg->rows+9,grayImg->cols+9,CV_32FC1);
	CvMat* tmp_pad13x13         = cvCreateMatHeader(grayImg->rows+6,grayImg->cols+6,CV_32FC1); 
	CvMat* tmp_pad19x19         = cvCreateMatHeader(grayImg->rows+9,grayImg->cols+9,CV_32FC1);
	for(int i=0; i< 24; i++)
		imgResponse[i] = cvCreateMat(grayImg->rows,grayImg->cols,CV_32FC1);

	cvCopyMakeBorder(grayImg,grayImg_pad13x13,cvPoint(6,6),IPL_BORDER_REPLICATE);
	cvCopyMakeBorder(grayImg,grayImg_pad19x19,cvPoint(9,9),IPL_BORDER_REPLICATE);
	
	float tmpVal;				
	for(int i=0; i< 24; i++){
		if (i < 12){
			//fread(tmp1,sizeof(double),13*13,fp);			
			for(int r=0; r < 13; r++){
				for(int c=0; c<13; c++){
					
					fscanf(fp_13,"%f",&tmpVal);
					cvSetReal2D(filterBank13x13,r,c,tmpVal);
				}
			}

			// step 2: convolve and get the response..	
			cvFilter2D(grayImg_pad13x13,imgResponse_pad13x13,filterBank13x13,cvPoint(6,6));
			cvGetSubRect(imgResponse_pad13x13,tmp_pad13x13,cvRect(6,6,imgResponse[i]->cols,imgResponse[i]->rows));
			cvCopy(tmp_pad13x13,imgResponse[i]);
		}
		else{
			//fread(tmp2,sizeof(double),19*19,fp);
			for(int r=0; r < 19; r++){
				for(int c=0; c<19; c++){
					fscanf(fp_19,"%f",&tmpVal);
					cvSetReal2D(filterBank19x19,r,c,tmpVal);
				}
			}
			// step 2: convolve and get the response..
			cvFilter2D(grayImg_pad19x19,imgResponse_pad19x19, filterBank19x19, cvPoint(9,9));
			cvGetSubRect(imgResponse_pad19x19,tmp_pad19x19,cvRect(9,9,imgResponse[i]->cols,imgResponse[i]->rows));
			cvCopy(tmp_pad19x19,imgResponse[i]);
		}	
	}
	fclose(fp_13);
	fclose(fp_19);
	cvReleaseMat(&imgResponse_pad13x13);
	cvReleaseMat(&imgResponse_pad19x19);
	cvReleaseMat(&grayImg_pad13x13);
	cvReleaseMat(&grayImg_pad19x19);
	cvReleaseMat(&filterBank13x13);
	cvReleaseMat(&filterBank19x19);




	// Read the textons
	strcpy(filterBankFileName, filterBin_dirLoc);
	strcat(filterBankFileName,"/textons.txt");
	FILE *fp = fopen(filterBankFileName,"r");
	//fp = fopen("textons.bin","rb");
	//double* tmp3 = (double*)malloc(64*24*sizeof(double));
	if (fp == NULL){
		fprintf(stderr,"textons.txt not found");
		exit(1);
	}
	//fread(tmp3, sizeof(double),64*24, fp);
	CvMat*  textons = cvCreateMat(64,24,CV_32FC1);

	for(int r=0; r < 64;r++){
		for(int c=0; c < 24; c++){			
			fscanf(fp,"%f",&tmpVal);
			cvSetReal2D(textons,r,c,tmpVal);
		}
	}
	fclose(fp);
	//free(tmp3);

	// find the closest texton for every pixel !!
	CvMat* colFrmTextonMat = cvCreateMatHeader(64,1,CV_32FC1);
	CvMat* totSum          = cvCreateMat(64,1,CV_32FC1);
	CvMat* tmp			   = cvCreateMat(64,1,CV_32FC1);
	for(int r=0; r<grayImg->rows; r++){
		for(int c=0; c<grayImg->cols; c++){
			cvSetZero(totSum);
			for(int i=0; i<24; i++){
				//find the distance of the pixel response from all other 
				cvGetCol(textons,colFrmTextonMat, i);
				cvSubS(colFrmTextonMat,cvGet2D(imgResponse[i],r,c),tmp);
				cvPow(tmp,tmp,2.0);
				cvAdd(totSum,tmp,totSum);
			}
			// find the texton with minimum distance
			CvPoint minLoc, maxLoc;
			double minVal, maxVal;
			cvMinMaxLoc(totSum,&minVal,&maxVal,&minLoc,&maxLoc);
			cvSetReal2D(tmap,r,c,(double)minLoc.y);
		}
	}

	//free matrices
	for(int i=0; i< 24; i++)
		cvReleaseMat(&imgResponse[i]);
	cvReleaseMat(&totSum);
	cvReleaseMat(&tmp);
	cvReleaseMat(&textons);
}

//-------------------------------------------------------
//-------------------------------------------------------
CvMat** detTG(IplImage* im, int norient, double* gtheta){

	double imDiag    = sqrt((double)((im->height)*(im->height)+(im->width)*(im->width)));
	
	// RGB -> gray conversion!
	IplImage* imgGray = cvCreateImage(cvSize(im->width,im->height),im->depth,1);
	cvCvtColor(im,imgGray,CV_BGR2GRAY);
	cvConvertScale(imgGray,imgGray,1.0/255); // gray scale values between [0 and 1]
	CvMat* imgGrayMat = cvCreateMatHeader(im->height,im->width,CV_32FC1);
	imgGrayMat = cvGetMat(imgGray,imgGrayMat);
	cvReleaseImage(&imgGray);

	// Assign textons
	CvMat*   tmap = cvCreateMat(im->height,im->width,CV_32FC1);
	assignTextons(tmap, imgGrayMat);

	int     ntex  = 64;
	double sigma  = imDiag*0.02;
	//int   norient =  8;
	fprintf(stdout,"\n\ttexture:");
	CvMat*   tsim = cvCreateMatHeader(im->height,im->width,CV_32FC1);
	CvMat**  tg   = tgmo(*tmap, ntex, sigma, gtheta, norient, *tsim, 1); 


	cvReleaseMat(&tmap);
	return tg;
}
