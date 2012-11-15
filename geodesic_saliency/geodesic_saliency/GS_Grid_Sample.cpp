#pragma once
#include "windows.h"
#include "stdio.h"
#include <Gdiplus.h>
using namespace Gdiplus;

#include "BGMapExtractor_Grid.h"

bool GetEncoderClsidPriv(const WCHAR* format, CLSID* pClsid)
{
    UINT num, size;
    Gdiplus::GetImageEncodersSize(&num, &size);
    Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
    bool found = false;
    for (UINT ix = 0; !found && ix < num; ++ix) {
        if (0 == _wcsicmp(pImageCodecInfo[ix].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[ix].Clsid;
            found = true;
        }
    }
    free(pImageCodecInfo);
    return found;
}

void SaveFloatImage(wchar_t *wzFileName, float *pfImage, int w, int h, int stride, CLSID *encoder, float fScale)
{
    BYTE *pColorPixels = new BYTE[h * stride];

    for (int y = 0; y < h; y++)
    {
        BYTE *pRow = pColorPixels + y * stride;
        float *pfOrgRow = pfImage + y * w;
        for (int x = 0; x < w; x++)
        {
            BYTE byValue = (BYTE)(max(0.f, min(255.f, pfOrgRow[x] * fScale + 0.5f)));
            pRow[x * 3] = pRow[x * 3 + 1] = pRow[x * 3 + 2] = byValue;
        }
    }
    Bitmap bmp(w, h, stride, PixelFormat24bppRGB, pColorPixels);
    bmp.Save(wzFileName, encoder, NULL);
    delete[] pColorPixels;
}

int wmain(int argc, wchar_t *argv[])
{
    if (argc < 3)
    {
        printf("[Usage:]GS_Grid_Sample.exe source_image saliency_map_image\n");
        return -1;
    }

    //read image from file
    Bitmap bmp(argv[1]);
    BitmapData bmpData;
    bmp.LockBits(NULL, ImageLockModeRead, PixelFormat24bppRGB, &bmpData);	// input rgb image

    int w = bmp.GetWidth();
    int h = bmp.GetHeight();
    int stride = bmpData.Stride;

    //initialize BGMap extractor
    //note that if we have multiple images to process, only need to initialize it once, 
    //but the iMaxWidth and iMaxHeight should be set to the maximum size of all images
    //for the grid size (iPatchWidth and iPatchHeight), usually if the image size is about 400*400, we can set the grid size to 10*10
    BGMapExtractor_Grid bgExtractor;
	int iMaxWidth = w;
	int iMaxHeight = h;
	int iPatchWidth = 10;
	int iPatchHeight = 10;

    if (!bgExtractor.Initialize(iMaxWidth, iMaxHeight, iPatchWidth, iPatchHeight))
    {
        printf("bgExtractor initialization error.\n");
        bmp.UnlockBits(&bmpData);
        return -1;
    }

    //allocate memory for saliency map
    float *pfBGMap = NULL;
    pfBGMap = new float[w * h];
    if (NULL == pfBGMap)
    {
        printf("memory error.\n");
        bmp.UnlockBits(&bmpData);
        return -1;
    }
    //generate saliency map (saliency values have been normalized to 0-1)
    if (!bgExtractor.BGMapExtract((const unsigned char *)bmpData.Scan0, w, h, stride, pfBGMap))
    {
        printf("BG_map extraction error.\n");
        bmp.UnlockBits(&bmpData);
        delete[] pfBGMap;
        return -1;
    }
    
    //save saliency map
    CLSID encoder;
    GetEncoderClsidPriv(L"mime/bmp", &encoder);
    SaveFloatImage(argv[2], pfBGMap, w, h, stride, &encoder, 255.f);

    delete[] pfBGMap;
    bmp.UnlockBits(&bmpData);

    return 0;
}
