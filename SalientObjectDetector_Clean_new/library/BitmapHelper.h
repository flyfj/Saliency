#pragma once

#include <windows.h>
#include <Gdiplus.h>
using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// format values : L"image/jpeg", L"image/bmp"
bool SaveBitmap(Bitmap& bitmap, const WCHAR* format, const WCHAR* filename);