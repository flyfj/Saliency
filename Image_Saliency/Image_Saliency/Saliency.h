#pragma once

struct Saliency
{
	// Get saliency values of a group of images.
	// Input image names and directory name for saving saliency maps.
	static void Get(const string &imgNameW, const string &salDir);

	// Evaluate saliency detection methods. Input ground truth file names and saliency map directory
	static void Evaluate(const string gtImgW, const string &salDir, const string &resName);

	// Frequency Tuned [1].
	static Mat GetFT(const Mat &img3f);

	// Histogram Contrast of [3]
	static Mat GetHC(const Mat &img3f);

	// Region Contrast 
	static Mat GetRC(const Mat &img3f);
	static Mat GetRC(const Mat &img3f, double sigmaDist, double segK, int segMinSize, double segSigma);

	// Luminance Contrast [2]
	static Mat GetLC(const Mat &img3f);

	// Spectral Residual [4]
	static Mat GetSR(const Mat &img3f);

	// Color quantization
	static int Quantize(const Mat& img3f, Mat &idx1i, Mat &_color3f, Mat &_colorNum, double ratio = 0.95);

private:
	static const int SAL_TYPE_NUM = 5;
	typedef Mat (*GET_SAL_FUNC)(const Mat &);
	static const char* SAL_TYPE_DES[SAL_TYPE_NUM];
	static const GET_SAL_FUNC gFuns[SAL_TYPE_NUM];

	// Histogram based Contrast
	static void GetHC(const Mat &binColor3f, const Mat &weights1f, Mat &colorSaliency);

	static void Evaluate(const string& resultW, const string &gtImgW, vecD &precision, vecD &recall);
	static int PrintVector(FILE *f, const vecD &v, const string &name, int maxNum = 1000);

	static void SmoothSaliency(const Mat &binColor3f, Mat &sal1d, float delta, const vector<vector<CostfIdx>> &similar);

	static void AbsAngle(const Mat& cmplx32FC2, Mat& mag32FC1, Mat& ang32FC1);
	static void GetCmplx(const Mat& mag32F, const Mat& ang32F, Mat& cmplx32FC2);


	struct Region{
		Region() { pixNum = 0;}
		int pixNum;  // Number of pixels
		vector<CostfIdx> freIdx;  // Frequency of each color and its index
		Point2d centroid;
	};
	static void BuildRegions(const Mat& regIdx1i, vector<Region> &regs, const Mat &colorIdx1i, int colorNum);
	static void RegionContrast(const vector<Region> &regs, const Mat &color3fv, Mat& regSal1d, double sigmaDist);
};

/************************************************************************/
/*[1]R. Achanta, S. Hemami, F. Estrada and S. Susstrunk, Frequency-tuned*/
/*   Salient Region Detection, IEEE International Conference on Computer*/
/*	 Vision and Pattern Recognition (CVPR), 2009.						*/
/*[2]Y. Zhai and M. Shah. Visual attention detection in video sequences */
/*   using spatiotemporal cues. In ACM Multimedia, pages 815¨C824. ACM, */
/*   2006.																*/
/*[3]Our paper															*/
/*[4]X. Hou and L. Zhang. Saliency detection: A spectral residual		*/
/*   approach. In IEEE Conference on Computer Vision and Pattern		*/
/*	 Recognition, 2007. CVPR¡¯07, pages 1¨C8, 2007.						*/
/************************************************************************/
