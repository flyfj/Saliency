/////////////////////////////////////////////////////////////////////////


#include "SalientObjSegmentor.h"
#include <fstream>
using namespace std;

int main()
{
	string imgdir = "D:\\test_samp\\cut\\test\\";	//BSDS300\\images\\test\\";
	string savepath = "D:\\test_samp\\cut\\res\\";	//bsds300_res2\\";
	SalientObjSegmentor segmentor;

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
		
		cout<<endl;
		// process each image
		Mat img = imread(imgpath.c_str());
		// resize to speed up
		Mat img_rz(img.rows/2, img.cols/2, img.depth());
		resize(img, img_rz, img_rz.size());
		imshow("img", img_rz);
		waitKey(10);

		if( segmentor.Init(img_rz, savepath, fname) )
			segmentor.RunSegmentation(img_rz);
		else
			cout<<"Not process image "<<fname<<endl;

		// get next image
		done = _findnext(handle, &ffblk);
	}
	
	waitKey(10);

	return 0;
}