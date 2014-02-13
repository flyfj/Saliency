/*
 *  textureGradient.h
 *  Created by Ajay Mishra on Oct 10, 2009
 *  Copyright 2009 University of Maryland at College Park. All rights reserved.
 */


#ifndef _textureGradient_H
#define _textureGradient_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>

void assignTextons(CvMat* , CvMat*);
CvMat** detTG(IplImage* , int, double* );

#endif
