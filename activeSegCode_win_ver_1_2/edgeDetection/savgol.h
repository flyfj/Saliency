/*
 *  savgol.h
 *  
 *
 *  Created by Leonardo Claudino on 8/3/09.
 *  Modified by Ajay Mishra on Oct 10, 2009
 *  Copyright 2009 University of Maryland at College Park. All rights reserved.
 *
 */


#ifndef _SAVGOL_H
#define _SAVGOL_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>

CvMat *savgolFilter(CvMat &, double, double, double);
CvMat *fitparab(CvMat &, double, double, double);
CvMat *tgso (CvMat &, int, double, double, CvMat &, int);
CvMat **tgmo(CvMat &, int, double, double*&, int, CvMat &, int);
CvMat **cgmo (IplImage* , int , double* );
void  pbCGTG (IplImage *,IplImage*, IplImage*);
void  pbCGTG_NoMaxSup (IplImage *,IplImage*, IplImage*);
void  pbCG (IplImage *,IplImage*, IplImage*);
void  pbBG (IplImage *,IplImage*, IplImage*);
CvMat *colorsim(int, double);
void pbCG(IplImage *, IplImage* , IplImage* );
CvMat** detCG(IplImage*, int, double*);
CvMat** detBG(IplImage*, int, double*);
void detCGTG(IplImage* , int , double* , CvMat** , CvMat** );
CvMat* nonmaxsup(CvMat* , double );

#define max(a, b) ((a)>(b)?(a):(b))

#ifndef M_PI
#define M_PI acos(-1.0)
#endif

#endif
