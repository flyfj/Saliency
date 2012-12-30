#include "CompositionCost.h"
#include "SalientObjSegmentor.h"
#include <fstream>
#include <io.h>
#include <direct.h>
using namespace std;

int main()
{

	string imgdir = "D:\\test_samp\\cut\\BSDS300\\images\\test\\";
	string maskpath = "D:\\test_samp\\cut\\bsds300_eccv_mask\\";
	string savepath = "D:\\test_samp\\cut\\bsds300_eccv_res\\";

	CompositionCost cc;

	// read images from directory
	struct _finddata_t ffblk;
	int done = 1;
	string imgfiles = imgdir + "*.jpg";
	int handle = _findfirst(imgfiles.c_str(), &ffblk);
	while( done != -1 )
	{
		string fname(ffblk.name);
		fname = fname.substr(0, fname.length()-4);	// image name without extension
		string imgpath = imgdir + fname + ".jpg";

		// process each image
		Mat img = imread(imgpath.c_str());
		Mat img_rz(img.rows/2, img.cols/2, img.depth());
		resize(img, img_rz, img_rz.size());
		cc.Init(img_rz);

		vector<SegmentResult> segmentRes;
		// read all masks of current image and compute score
		string maskdir = maskpath + fname + "\\";
		struct _finddata_t ffblk1;
		int done1 = 1;
		string maskfiles = maskdir + "*.jpg";
		int handle1 = _findfirst(maskfiles.c_str(), &ffblk1);
		int cnt = 0;
		while( done1 != -1 )
		{
			string mname(ffblk1.name);
			string maskimg = maskdir + mname;
			Mat curmask = imread(maskimg.c_str(), 0);
			float area = countNonZero(curmask);
			if(area/(curmask.rows*curmask.cols) > 0.5f)	// skip too big mask
			{
				done1 = _findnext(handle1, &ffblk1);
				continue;
			}

			cnt++;
			SegmentResult mask;
			mask.mask.create(img_rz.size(), CV_8U);
			resize(curmask, mask.mask, img_rz.size());

			mask.obj_img.create(img_rz.size(), img_rz.depth());
			img_rz.copyTo(mask.obj_img, mask.mask);
			mask.CC_score = cc.ComputeCompositionCost(mask.mask);
			cout<<"mask "<<cnt<<" done."<<endl;
			segmentRes.push_back(mask);

			// get next mask
			done1 = _findnext(handle1, &ffblk1);
		}

		// rank and do nms
		sort(segmentRes.begin(), segmentRes.end(), SalientObjSegmentor::rank_by_cc);

		// remove overlapped segments with lower score
		Mat finalMask(img_rz.size(), CV_8U);
		for(size_t i=0; i<segmentRes.size(); i++)	// from high score to low
		{
			if( !segmentRes[i].m_bValid )
				continue;

			for(size_t j=i+1; j<segmentRes.size(); j++)
			{
				if( !segmentRes[j].m_bValid )
					continue;

				// compute intersection/union
				float intersectArea = 0;
				float unionArea = 0;
				// intersection
				bitwise_and( segmentRes[i].mask, segmentRes[j].mask, finalMask );
				intersectArea = countNonZero(finalMask);
				// union
				bitwise_or( segmentRes[i].mask, segmentRes[j].mask, finalMask );
				unionArea = countNonZero(finalMask);
				if(intersectArea / unionArea > 0.8f)	// remove low score segment
					segmentRes[j].m_bValid = false;
			}
		}

		// save to dir
		string savedir = savepath + fname;
		mkdir(savedir.c_str());
		char str[100];
		cnt = -1;
		for(size_t i=0; i<segmentRes.size(); i++)
		{
			if(!segmentRes[i].m_bValid)
				continue;

			cnt++;
			string maskPath;
			maskPath = savedir + "\\" + fname + "_%d_mask.jpg";
			sprintf(str, maskPath.c_str(), cnt);
			imwrite(str, segmentRes[i].mask);
			// save object
			string objPath = savedir + "\\" + fname + "_%d_obj.jpg";
			sprintf(str, objPath.c_str(), cnt);
			imwrite(str, segmentRes[i].obj_img);
		}
		

		// get next image
		done = _findnext(handle, &ffblk);
	}

	return 0;


	/*Mat img = imread("D:\\test_samp\\cut\\test\\85048.jpg");
	imshow("img", img);
	waitKey(10);
	Mat newimg(img.rows/2, img.cols/2, img.depth());
	resize(img, newimg, newimg.size());
	
	Mat mask1 = imread("D:\\test_samp\\cut\\res\\85048\\85048_156_mask.jpg", 0);
	Mat mask2 = imread("D:\\test_samp\\cut\\res\\85048\\85048_0_mask.jpg", 0);

	CompositionCost cc;
	cc.Init(newimg);

	float val1 = cc.ComputeCompositionCost(mask1);
	float val2 = cc.ComputeCompositionCost(mask2);

	cout<<val1<<" "<<val2<<endl;

	return 0;*/
}