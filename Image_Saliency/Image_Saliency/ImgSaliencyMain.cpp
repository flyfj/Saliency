// ImgSaliency.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Saliency.h"

bool SaliencyValidParameters(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	if (!SaliencyValidParameters(argc, argv))
		return 1;

	string imgNameW = argv[1];
	string gtImgW = argv[2];
	string resultFileName = argv[4];
	string salDir = string(argv[3]) + "\\";

	CmLog::LogLine(FOREGROUND_GREEN,"Get saliency maps for images `%s' and save results to `%s'\n", imgNameW.c_str(), salDir.c_str());
	Saliency::Get(imgNameW, salDir);

	CmLog::LogLine(FOREGROUND_GREEN, "Evaluate saliency maps according to ground truth results `%s'\n", gtImgW.c_str());
	Saliency::Evaluate(gtImgW, salDir, resultFileName);
	CmLog::LogLine(FOREGROUND_GREEN, "Results saved to %s\n", resultFileName.c_str());
	return 0;
}


bool SaliencyValidParameters(int argc, char* argv[])
{
	if (argc != 5)
	{
		printf("Usage: ImgSaliency inImgs tImgs salDir result\n");
		printf("\tinImgs:\tInput images. e.g. `D:\\Src\\*.jpg'\n");
		printf("\ttImgs:\tGround truth results. e.g. `D:\\SrC\\*.bmp'\n");
		printf("\tsalDir:\tSaliency map saving directory. e.g. `D:\\Sal'\n");
		printf("\tresult:\tStatistic result show name. e.g. `D:\\EvaluateShow.m'\n");
		return false;
	}

	CmLog::Set("SalLog.txt", false);
	return true;
}
