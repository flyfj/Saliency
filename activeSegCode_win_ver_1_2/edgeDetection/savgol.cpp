/*
*  savgol.cpp
*  Created by Leonardo Claudino on 8/3/09.
*  Modified by Ajay Mishra on Oct 05, 2009
*  Modified by Ajay Mishra on Oct 21, 2009 (function tgmo Multithreaded..)
*  Modified by Ajay Mishra on Mar 31, 2011 (multhreading removed for windows!)
*  Modified by Ajay Mishra on Oct 06, 2011 (function to convert, rgb to lab image, cvCvtColor has a bug in ver 2.3)
*/

#include "savgol.h"
#include "textureGradient.h"
#include <windows.h>
#include <process.h>

//----------------------------------------------------------------------------
inline float f(float t){
	return ((t > 0.008856) ? pow((double)t,(double)1.0/3.0):7.787*t+16.0/116);
}
//-----------------------------------------------------------------------------
// BGR to Lab Coversion
//-----------------------------------------------------------------------------
void myConvertBGR2Lab(IplImage* imgBGR_float, IplImage* imgLab_float){
	for(int i=0; i<imgBGR_float->height; i++){
		for(int j=0; j<imgBGR_float->width; j++){
			float R = CV_IMAGE_ELEM(imgBGR_float, float, i, 3*j+2);
			float G = CV_IMAGE_ELEM(imgBGR_float, float, i, 3*j+1);
			float B = CV_IMAGE_ELEM(imgBGR_float, float, i, 3*j);		
			float X = (0.412453*R + 0.357580*G + 0.180423*B)/0.950456;
			float Y = 0.212671*R + 0.715160*G + 0.072169*B;
			float Z = (0.019334*R + 0.119193*G + 0.950227*B)/1.088754;
			
			float L = (Y > 0.008856) ? 116*pow((double)Y,(double)1.0/3.0)-16: 903.3*Y;
			float a = 500*(f(X)-f(Y));
			float b = 200*(f(Y)-f(Z));
			
			CV_IMAGE_ELEM(imgLab_float, float, i, j*3) 	= L;
			CV_IMAGE_ELEM(imgLab_float, float, i, j*3+1) = a;				
			CV_IMAGE_ELEM(imgLab_float, float, i, j*3+2) = b;
		}
	}	
	
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
double round(double x){
	double y;
	if(x+0.5 > ceil(x))
		y = ceil(x);
	else 
		y = floor(x);
	return y;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CvMat *savgolFilter(CvMat &z, double ra, double rb, double theta){

	int d=2;
	int k=1; // Always these, according to the matlab code (Leo)
	ra = max(1.5,ra);
	rb = max(1.5,rb);
	double ira2 = 1.0/(ra*ra);
	double irb2 = 1.0/(rb*rb);
	double wr = floor(max(ra,rb));
	double wd = 2*wr+1;
	double sint = sin(theta);
	double cost = cos(theta);

	CvMat*xx= cvCreateMat(2*d+1, 1, CV_64FC1);
	cvSetZero(xx);
	CvMat *temp=cvCreateMat(2*d+1, 1, CV_64FC1);

	for (int u=-wr;u<=wr;u++)
		for (int v=-wr;v<=wr;v++) {
			double ai=-u*sint+v*cost; //distance along major axis
			double bi= u*cost+v*sint; //distance along minor axis

			if (ai*ai*ira2+bi*bi*irb2 <= 1) {
				cvSet(temp, cvScalar(ai));
				cvSetReal2D(temp, 0, 0, 1.0);

				double dTemp=1;

				for (int i=0;i<2*d+1-1;i++) {
					dTemp=dTemp*cvGetReal2D(temp,i+1,0);
					cvSetReal2D(temp,i+1,0,dTemp);
				}

				cvAdd(xx,temp,xx);

			}
		}

		cvReleaseMat(&temp);

		CvMat *A=cvCreateMat(d+1,d+1,CV_64FC1);
		for (int i=0;i<d+1;i++)
			for (int j=i;j<=i+d;j++)
				cvSetReal2D(A,j-i,i,cvGetReal2D(xx,j,0));

		cvInvert(A,A,CV_LU);

		CvMat *zz=cvCreateMat(wd,wd, CV_64FC1);
		CvMat *yy=cvCreateMat(d+1,1, CV_64FC1);
		CvMat *result=cvCreateMat(d+1,1, CV_64FC1);
		CvMat *filt=cvCreateMat(wd,wd, CV_32FC1);
		cvSetZero(filt);


		for (int u=-wr;u<=wr;u++)
			for (int v=-wr;v<=wr;v++) {

				cvSetZero(zz);
				cvSetReal2D(zz,v+wr,u+wr,1);
				cvSetZero(yy);

				double ai=-u*sint+v*cost; //distance along major axis
				double bi= u*cost+v*sint; //distance along minor axis

				if (ai*ai*ira2+bi*bi*irb2 <= 1) {
					cvSet(yy, cvScalar(ai));
					cvSetReal2D(yy, 0, 0, 1.0);

					double dTemp=1;
					for (int i=0;i<d+1-1;i++) {
						dTemp=dTemp*cvGetReal2D(yy,i+1,0);
						cvSetReal2D(yy,i+1,0,dTemp);
					}

					cvMatMul(A,yy,result);
					cvSetReal2D(filt,v+wr,u+wr,cvGetReal2D(result,k-1,0));
				}
			}

			cvReleaseMat(&zz);
			cvReleaseMat(&yy);
			cvReleaseMat(&xx);
			cvReleaseMat(&A);
			cvReleaseMat(&result);

			CvMat *ztemp= cvCreateMat(z.rows+filt->rows-1,z.cols+filt->cols-1,CV_32FC1);
			cvCopyMakeBorder(&z,ztemp,cvPoint((filt->cols-1)/2,(filt->rows-1)/2),IPL_BORDER_CONSTANT);
			CvMat *filteredtemp= cvCreateMat(ztemp->rows,ztemp->cols,CV_32FC1);

			cvFilter2D(ztemp,filteredtemp,filt,cvPoint((filt->cols-1)/2,(filt->rows-1)/2));

			CvMat *filtered = cvCreateMatHeader(z.rows,z.cols,CV_32FC1);
			cvGetSubRect(filteredtemp,filtered,cvRect((filt->cols-1)/2,(filt->rows-1)/2,z.cols,z.rows));
			
			cvReleaseMat(&ztemp);
				
			return filtered;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CvMat *fitparab(CvMat &z, double ra, double rb, double theta) {

	ra = max(1.5,ra);
	rb = max(1.5,rb);
	double wr = floor(max(ra,rb));
	CvMat *filtered = savgolFilter(z,ra,rb,theta);

	return filtered;	
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CvMat *tgso (CvMat &tmap, int ntex, double sigma, double theta, CvMat &tsim, int useChi2) {


	CvMat *roundTmap=cvCreateMat(tmap.rows,tmap.cols,CV_32FC1);
	CvMat *comp=cvCreateMat(tmap.rows,tmap.cols,CV_32FC1);

	for (int i=0;i<tmap.rows;i++)
		for (int j=0;j<tmap.cols;j++)
			cvSetReal2D(roundTmap,i,j,cvRound(cvGetReal2D(&tmap,i,j)));

	cvSub(&tmap,roundTmap,comp);
	if (cvCountNonZero(comp)) {
		printf("texton labels not integral");
		cvReleaseMat(&roundTmap);
		cvReleaseMat(&comp);
		exit(1);
	}

	double min,max;
	cvMinMaxLoc(&tmap,&min,&max);
	if (min<1 && max>ntex) {
		char *msg=new char[50];
		printf(msg,"texton labels out of range [1,%d]",ntex);
		cvReleaseMat(&roundTmap);
		cvReleaseMat(&comp);
		exit(1);
	}

	cvReleaseMat(&roundTmap);
	cvReleaseMat(&comp);


	double wr=floor(sigma); //sigma=radius (Leo) 

	CvMat *x=cvCreateMat(1,wr-(-wr)+1, CV_64FC1);
	CvMat *y=cvCreateMat(wr-(-wr)+1,1, CV_64FC1);

	CvMat *u=cvCreateMat(wr-(-wr)+1,wr-(-wr)+1, CV_64FC1);
	CvMat *v=cvCreateMat(wr-(-wr)+1,wr-(-wr)+1, CV_64FC1);
	CvMat *gamma=cvCreateMat(u->rows,v->rows, CV_64FC1);

	// Set x,y directions 
	for (int j=-wr;j<=wr;j++) {
		cvSetReal2D(x,0,(j+wr),j);
		cvSetReal2D(y,(j+wr),0,j);
	}

	// Set u,v, meshgrids
	for (int i=0;i<u->rows;i++) {
		cvRepeat(x,u);
		cvRepeat(y,v);
	}

	// Compute the gamma matrix from the grid
	for (int i=0;i<u->rows;i++) 
		for (int j=0;j<u->cols;j++)
			cvSetReal2D(gamma,i,j,atan2(cvGetReal2D(v,i,j),cvGetReal2D(u,i,j)));

	cvReleaseMat(&x);
	cvReleaseMat(&y);

	CvMat *sum=cvCreateMat(u->rows,u->cols, CV_64FC1);
	cvMul(u,u,u);
	cvMul(v,v,v);
	cvAdd(u,v,sum);
	CvMat *mask=cvCreateMat(u->rows,u->cols, CV_8UC1);
	cvCmpS(sum,sigma*sigma,mask,CV_CMP_LE);
	cvConvertScale(mask,mask,1.0/255);
	cvSetReal2D(mask,wr,wr,0);
	int count=cvCountNonZero(mask);

	cvReleaseMat(&u);
	cvReleaseMat(&v);
	cvReleaseMat(&sum);

	CvMat *sub=cvCreateMat(mask->rows,mask->cols, CV_64FC1);
	CvMat *side=cvCreateMat(mask->rows,mask->cols, CV_8UC1);

	cvSubS(gamma,cvScalar(theta),sub);
	cvReleaseMat(&gamma);

	for (int i=0;i<mask->rows;i++){
		for (int j=0;j<mask->cols;j++) {
			double n=cvmGet(sub,i,j);
			double n_mod = n-floor(n/(2*M_PI))*2*M_PI;
			cvSetReal2D(side,i,j, 1 + int(n_mod < M_PI));
		}
	}

	cvMul(side,mask,side);
	cvReleaseMat(&sub);
	cvReleaseMat(&mask);

	CvMat *lmask=cvCreateMat(side->rows,side->cols, CV_8UC1);
	CvMat *rmask=cvCreateMat(side->rows,side->cols, CV_8UC1);
	cvCmpS(side,1,lmask,CV_CMP_EQ);
	cvCmpS(side,2,rmask,CV_CMP_EQ);
	int count1=cvCountNonZero(lmask), count2=cvCountNonZero(rmask);
	if (count1 != count2) {
		printf("Bug: imbalance\n");
	}

	CvMat *rlmask=cvCreateMat(side->rows,side->cols, CV_32FC1);
	CvMat *rrmask=cvCreateMat(side->rows,side->cols, CV_32FC1);
	cvConvertScale(lmask,rlmask,1.0/(255*count)*2);
	cvConvertScale(rmask,rrmask,1.0/(255*count)*2);


	cvReleaseMat(&lmask);
	cvReleaseMat(&rmask);
	cvReleaseMat(&side);

	int h=tmap.rows;
	int w=tmap.cols;


	CvMat *d       = cvCreateMat(h*w,ntex,CV_32FC1);
	CvMat *coltemp = cvCreateMat(h*w,1,CV_32FC1);
	CvMat *tgL     = cvCreateMat(h,w, CV_32FC1);
	CvMat *tgR     = cvCreateMat(h,w, CV_32FC1);
	CvMat *temp    = cvCreateMat(h,w,CV_8UC1);
	CvMat *im      = cvCreateMat(h,w, CV_32FC1);
	CvMat *sub2    = cvCreateMat(h,w,CV_32FC1);
	CvMat *sub2t   = cvCreateMat(w,h,CV_32FC1);
	CvMat *prod    = cvCreateMat(h*w,ntex,CV_32FC1);
	CvMat reshapehdr,*reshape;

	CvMat* tgL_pad = cvCreateMat(h+rlmask->rows-1,w+rlmask->cols-1,CV_32FC1);
	CvMat* tgR_pad = cvCreateMat(h+rlmask->rows-1,w+rlmask->cols-1,CV_32FC1);
	CvMat* im_pad  = cvCreateMat(h+rlmask->rows-1,w+rlmask->cols-1,CV_32FC1);

	CvMat *tg=cvCreateMat(h,w,CV_32FC1);
	cvZero(tg);

	if (useChi2 == 1){
		CvMat* temp_add1 = cvCreateMat(h,w,CV_32FC1);
		for (int i=0;i<ntex;i++) {
			cvCmpS(&tmap,i+1,temp,CV_CMP_EQ); 
			cvConvertScale(temp,im,1.0/255);

			cvCopyMakeBorder(tgL,tgL_pad,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2),IPL_BORDER_CONSTANT);
			cvCopyMakeBorder(tgR,tgR_pad,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2),IPL_BORDER_CONSTANT);
			cvCopyMakeBorder(im,im_pad,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2),IPL_BORDER_CONSTANT);

			cvFilter2D(im_pad,tgL_pad,rlmask,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2));
			cvFilter2D(im_pad,tgR_pad,rrmask,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2));

			cvGetSubRect(tgL_pad,tgL,cvRect((rlmask->cols-1)/2,(rlmask->rows-1)/2,tgL->cols,tgL->rows));
			cvGetSubRect(tgR_pad,tgR,cvRect((rlmask->cols-1)/2,(rlmask->rows-1)/2,tgR->cols,tgR->rows));

			cvSub(tgL,tgR,sub2);
			cvPow(sub2,sub2,2.0);
			cvAdd(tgL,tgR,temp_add1);
			cvAddS(temp_add1,cvScalar(0.0000000001),temp_add1);
			cvDiv(sub2,temp_add1,sub2);
			cvAdd(tg,sub2,tg);
		}
		cvScale(tg,tg,0.5);
		cvReleaseMat(&temp_add1);

	}
	else{// if not chi^2
		for (int i=0;i<ntex;i++) {
			cvCmpS(&tmap,i+1,temp,CV_CMP_EQ); 
			cvConvertScale(temp,im,1.0/255);

			cvCopyMakeBorder(tgL,tgL_pad,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2),IPL_BORDER_CONSTANT);
			cvCopyMakeBorder(tgR,tgR_pad,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2),IPL_BORDER_CONSTANT);
			cvCopyMakeBorder(im,im_pad,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2),IPL_BORDER_CONSTANT);

			cvFilter2D(im_pad,tgL_pad,rlmask,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2));
			cvFilter2D(im_pad,tgR_pad,rrmask,cvPoint((rlmask->cols-1)/2,(rlmask->rows-1)/2));

			cvGetSubRect(tgL_pad,tgL,cvRect((rlmask->cols-1)/2,(rlmask->rows-1)/2,tgL->cols,tgL->rows));
			cvGetSubRect(tgR_pad,tgR,cvRect((rlmask->cols-1)/2,(rlmask->rows-1)/2,tgR->cols,tgR->rows));

			cvSub(tgL,tgR,sub2);
			cvAbs(sub2,sub2);
			cvTranspose(sub2,sub2t);
			reshape=cvReshape(sub2t,&reshapehdr,0,h*w);
			cvGetCol(d,coltemp,i);
			cvCopy(reshape,coltemp);
		}

		cvMatMul(d,&tsim,prod);
		cvMul(prod,d,prod);


		CvMat *sumcols=cvCreateMat(h*w,1,CV_32FC1);
		cvSetZero(sumcols);
		for (int i=0;i<prod->cols;i++) {
			cvGetCol(prod,coltemp,i);
			cvAdd(sumcols,coltemp,sumcols);
		}

		reshape=cvReshape(sumcols,&reshapehdr,0,w);
		cvTranspose(reshape,tg);

		cvReleaseMat(&sumcols);
	}


	//Smooth the gradient now!!
	tg=fitparab(*tg,sigma,sigma/4,theta);
	cvMaxS(tg,0,tg); 


	cvReleaseMat(&im_pad);
	cvReleaseMat(&tgL_pad);
	cvReleaseMat(&tgR_pad);
	cvReleaseMat(&rlmask);
	cvReleaseMat(&rrmask);
	cvReleaseMat(&im);
	cvReleaseMat(&tgL);
	cvReleaseMat(&tgR);
	cvReleaseMat(&temp);
	cvReleaseMat(&coltemp);
	cvReleaseMat(&sub2);
	cvReleaseMat(&sub2t);
	cvReleaseMat(&d);
	cvReleaseMat(&prod);

	return tg;

}
CvMat **tgmo (CvMat &tmap, int ntex, double sigma, double *&theta, int norient, CvMat &tsim, int useChi2) {

	CvMat **tg=new CvMat*[norient];

	sigma   = max(1,sigma);
	norient = max(1,norient);

	for(int i=0; i <norient; i++){
		theta[i]=(double)i/norient*M_PI;
		tg[i] = tgso(tmap, ntex, sigma, theta[i], tsim, useChi2);
		fprintf(stdout," ."); fflush(stdout);
	}
	return tg;

}

//------------------------------------------------------------------------------
// Compute the color gradient at a single scale and multiple orientations
//------------------------------------------------------------------------------
CvMat **cgmo (IplImage* im, int norient, double* theta) {

	int   h  = im->height;
	int   w  = im->width;
	double imDiag = sqrt((double)(h*h+w*w));

	char    channelName[] = {'L', 'A', 'B'};

	int     nbins[3]     = {32, 32, 32};
	double sigmaSim[3]   = {0.1, 0.1, 0.1};
	int     nChannels    = im->nChannels;
	double sigmaSmo[3];
	sigmaSmo[0]          = imDiag*0.01;
	sigmaSmo[1]          = imDiag*0.02;
	sigmaSmo[2]          = imDiag*0.02;

	CvMat **Lab = new CvMat *[im->nChannels];

	// If RGB image, convert to Lab
	if (im->nChannels == 3) {
		double gamma=2.5; 
		double abmin=-73; 
		double abmax=95;
		IplImage *temp= cvCreateImage(cvGetSize(im),IPL_DEPTH_32F,3);
		cvConvertScale(im,temp,1.0/255);
		cvPow(temp,temp,gamma);
		myConvertBGR2Lab(temp,temp);	

		Lab[0]=cvCreateMat(im->height,im->width, CV_32FC1);
		Lab[1]=cvCreateMat(im->height,im->width, CV_32FC1);
		Lab[2]=cvCreateMat(im->height,im->width, CV_32FC1);
		cvSplit(temp, Lab[0], Lab[1], Lab[2], 0);

		// Setting L channel
		cvConvertScale(Lab[0], Lab[0], 1.0/100);

		// Setting a channel
		cvSubS(Lab[1],cvScalar(abmin),Lab[1]);
		cvConvertScale(Lab[1],Lab[1],1.0/(abmax-abmin));
		cvMinS(Lab[1],1,Lab[1]);
		cvMaxS(Lab[1],0,Lab[1]);

		// Setting b channel
		cvSubS(Lab[2],cvScalar(abmin),Lab[2]);
		cvConvertScale(Lab[2],Lab[2],1.0/(abmax-abmin));
		cvMinS(Lab[2],1,Lab[2]);
		cvMaxS(Lab[2],0,Lab[2]);

		cvReleaseImage(&temp);
	}
	else {
		printf("This is currently working with color images only.\n");
		exit(1);
	}

	norient = max(1,norient);

	for (int i=0;i<nChannels;i++)	
		nbins[i] = max(1,nbins[i]);

	for (int i=0;i<nChannels;i++) {
		double min, max;
		cvMinMaxLoc(Lab[i],&min,&max);
		if (min < 0 || max > 1) {
			printf("error -- pixel values out of range [0,1]\n");
			exit(1);
		}
		for (int j=0;j<nChannels;j++) {
			if (nbins[i] < 1.0/sigmaSim[j])
				printf("warning -- nbins < 1/sigmaSim is suspect\n");
		}

	}


	CvMat **cg = new CvMat *[norient*nChannels];

	for (int i=0;i<nChannels;i++) {
		CvMat *csim = colorsim (nbins[i], sigmaSim[i]);
		CvMat *cmap = cvCreateMat(Lab[i]->rows,Lab[i]->cols,CV_32FC1);
		cvConvertScale(Lab[i],cmap,nbins[i]);

		for (int k=0;k < Lab[i]->rows; k++){
			for (int l=0; l< Lab[i]->cols; l++){
				cvSetReal2D(cmap,k,l,max(1,ceil(cvGetReal2D(cmap,k,l))));
			}
		}

		fprintf(stdout,"\n\tChannel %c:",channelName[i]); fflush(stdout);
		CvMat **temp=tgmo(*cmap, nbins[i], sigmaSmo[i], theta, norient, *csim, 0);


		for (int o=0;o<norient;o++) {
			cg[i*norient+o]=cvCreateMat(Lab[0]->rows,Lab[0]->cols,CV_32FC1);
			cvCopy(temp[o],cg[i*norient+o]);
			cvReleaseMat(&temp[o]);
		}

		cvReleaseMat (&cmap);
		cvReleaseMat (&csim);
		delete temp;
	}

	cvReleaseMat(&Lab[0]);
	cvReleaseMat(&Lab[1]);
	cvReleaseMat(&Lab[2]);

	return cg;

}

//------------------------------------------------------------------------------
// Compute probability of boundary using Color gradient
//------------------------------------------------------------------------------
CvMat** detCG(IplImage* im, int norient, double* gtheta){
	//compute color gradient: cgmo
	CvMat** grad;
	grad = cgmo(im, norient, gtheta);
	return grad;
}

//-----------------------------------------------------------------------------
// Compute probability of boundary using brightness 
//-----------------------------------------------------------------------------
CvMat** detBG(IplImage* im, int norient, double* gtheta){
	// RGB -> gray conversion!
	IplImage* imgGray = cvCreateImage(cvSize(im->width,im->height),im->depth,1);
	cvCvtColor(im,imgGray,CV_BGR2GRAY);
	cvConvertScale(imgGray,imgGray,1.0/255); // gray scale values between [0 and 1]


	// set parameters!
	double imDiag    = sqrt((double)((im->height)*(im->height)+(im->width)*(im->width)));
	double sigma  	 = imDiag*0.01;
	double sigmaSim  = 0.1;
	int    nbins     = 32;

	//create label map
	CvMat*   bmap = cvCreateMat(im->height,im->width,CV_32FC1);
	cvConvertScale(imgGray, bmap, nbins);
	for (int k=0;k < bmap->rows; k++){
		for (int l=0; l< bmap->cols; l++){
			cvSetReal2D(bmap,k,l,max(1,ceil(cvGetReal2D(bmap,k,l))));
		}
	}

	CvMat   *bsim = colorsim (nbins, sigmaSim);

	fprintf(stdout,"\n\tBrightness: "); fflush(stdout);	
	CvMat**  bg   = tgmo(*bmap, nbins, sigma, gtheta, norient, *bsim, 0); 
	cvReleaseMat(&bsim);
	cvReleaseMat(&bmap);
	cvReleaseImage(&imgGray);

	return bg;
}

//------------------------------------------------------------------------------
// Color Similarity Matrix Calculation
//------------------------------------------------------------------------------
CvMat *colorsim(int nbins, double sigma) {

	CvMat *xc=cvCreateMat(1,nbins, CV_32FC1);
	CvMat *yr=cvCreateMat(nbins,1, CV_32FC1);

	CvMat *x=cvCreateMat(nbins,nbins, CV_32FC1);
	CvMat *y=cvCreateMat(nbins,nbins, CV_32FC1);
	CvMat *m=cvCreateMat(x->rows,x->rows, CV_32FC1);


	// Set x,y directions 
	for (int j=0;j<nbins;j++) {
		cvSetReal2D(xc,0,j,(j+1-0.5)/nbins);
		cvSetReal2D(yr,j,0,(j+1-0.5)/nbins);
	}

	// Set u,v, meshgrids
	for (int i=0;i<x->rows;i++) {
		cvRepeat(xc,x);
		cvRepeat(yr,y);
	}

	CvMat *sub = cvCreateMat(x->rows,y->cols,CV_32FC1);
	cvSub(x,y,sub);
	cvAbs(sub,sub);
	cvMul(sub,sub,sub);
	cvConvertScale(sub,sub,-1.0/(2*sigma*sigma));
	cvExp(sub,sub);
	cvSubRS(sub,cvScalar(1.0),m);

	cvReleaseMat(&xc);
	cvReleaseMat(&yr);
	cvReleaseMat(&x);
	cvReleaseMat(&y);
	cvReleaseMat(&sub);

	return m;
}


