#pragma once

#include <Windows.h>
#include <GdiPlus.h>
using namespace Gdiplus;

#include <ImageSimple.h>

// segmentedImg: 3*width*height bgr image buffer
// bmp: input bgr image; segmentedImg: output segmentation image for display
int graph_based_segment(const BitmapData& bmp, float sigma, float c, int min_size, ImageUIntSimple& indexImg, unsigned char* segmentedImg);