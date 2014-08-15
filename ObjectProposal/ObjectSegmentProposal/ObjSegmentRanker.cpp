#include "ObjSegmentRanker.h"

namespace objectproposal
{
	ObjSegmentRanker::ObjSegmentRanker(void)
	{
		features::ColorFeatParams cparams;
		cparams.feat_type = features::COLOR_FEAT_HIST;
		cparams.histParams.color_space = features::COLOR_RGB;
		colordesc.Init(cparams);



	}

	//////////////////////////////////////////////////////////////////////////

	float ObjSegmentRanker::ComputeCenterSurroundColorContrast(const Mat& cimg, const SuperPixel& sp)
	{
		// context window
		ImgWin spbox(sp.box.x, sp.box.y, sp.box.width, sp.box.height);
		ImgWin contextWin = tools::ToolFactory::GetContextWin(cimg.cols, cimg.rows, spbox, 2);
		Mat contextMask(cimg.rows, cimg.cols, CV_8U);
		contextMask.setTo(0);
		contextMask(contextWin).setTo(1);
		contextMask.setTo(0, sp.mask);

		// compute color descriptors
		Mat contextColor;
		Mat spColor;
		colordesc.Compute(cimg, contextColor, contextMask);
		colordesc.Compute(cimg, spColor, sp.mask);

		float score = norm(contextColor, spColor, NORM_L2);
		return score;
	}
}



