#pragma once

#include <iostream>
#include <img\Img_Image.h>
using namespace img;

int graph_based_segment(const CImageRgb& img, float sigma, float c, int min_size, CImage<unsigned int>& indexImg, CImageRgb& segmentedImg);

