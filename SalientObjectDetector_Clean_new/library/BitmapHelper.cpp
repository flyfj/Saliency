#include "BitmapHelper.h"

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using Gdiplus::ImageCodecInfo;
	using Gdiplus::GetImageEncodersSize;
	using Gdiplus::GetImageEncoders;

	UINT  num = 0;          /// number of image encoders
	UINT  size = 0;         /// size of the image encoder array in bytes

	/// get the number of codecs in list
	GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;  /// Failure
	if (size <= num * sizeof(ImageCodecInfo)) return -1;

	/// Allocate the memory, which maybe larger than ImageCodeInfo array
	BYTE* pBuffer = new BYTE[size];
	if (pBuffer == NULL) return -1;  /// Failure

	/// Bind the head of buffer to arrray of codec infos
	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*) (void*) pBuffer;

	/// read the codec info
	GetImageEncoders(num, size, pImageCodecInfo);

	/// enumerate and check the name
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			delete[] pBuffer;
			return j;  /// Success
		}    
	}

	delete[] pBuffer;
	return -1;  /// Failure
}

bool SaveBitmap(Bitmap& bitmap, const WCHAR* format, const WCHAR* filename)
{
	CLSID  encoderClsid;
	if (GetEncoderClsid(format, &encoderClsid) < 0)
		return false;
	Status s = bitmap.Save(filename, &encoderClsid);
	return s == Ok;
}