
#include "NYUDepth2DataMan.h"



//////////////////////////////////////////////////////////////////////////

bool NYUDepth2DataMan::GetImageList(FileInfos& imgfiles)
{
	ToolFactory::GetFilesFromDir(imgdir, "*.jpg", imgfiles);
	if (imgfiles.empty())
		return false;

	cout<<"Loaded NYU image list"<<endl;
	return true;
}

bool NYUDepth2DataMan::GetDepthmapList(FileInfos& depthfiles)
{
	FileInfos imgfns;
	GetImageList(imgfns);
	depthfiles.resize(imgfns.size());
	for(size_t i=0; i<imgfns.size(); i++)
	{
		depthfiles[i].filename = imgfns[i].filename.substr(0, imgfns[i].filename.length()-4) + "_d.txt";
		depthfiles[i].filepath = imgfns[i].filepath.substr(0, imgfns[i].filepath.length()-4) + "_d.txt";
	}

	cout<<"Loaded NYU depth map list"<<endl;
	return true;
}

bool NYUDepth2DataMan::LoadDepthData(const string& depthfile, cv::Mat& depthmap)
{
	ifstream in(depthfile);
	if( !in.is_open() )
		return false;

	int imgw, imgh;
	in>>imgw>>imgh;

	depthmap.create(imgh, imgw, CV_32F);
	for(int r=0; r<imgh; r++)
		for(int c=0; c<imgw; c++)
			in>>depthmap.at<float>(r, c);
	
	return true;
}

bool NYUDepth2DataMan::LoadGTMasks(const FileInfos& imgfiles, map<string, vector<Mat>>& gtmasks)
{
	gtmasks.clear();
	visualsearch::processors::ShapeAnalyzer shapetool;
	for (int i=0; i<imgfiles.size(); i++)
	{
		string labelfn = imgfiles[i].filepath.substr(0, imgfiles[i].filepath.length()-4) + "_l.png";
		Mat curlabelimg = imread(labelfn, CV_LOAD_IMAGE_UNCHANGED);
		curlabelimg.convertTo(curlabelimg, CV_32S);
		map<int, Mat> curmasks;
		for(int r=0; r<curlabelimg.rows; r++) for(int c=0; c<curlabelimg.cols; c++)
		{
			int curlabel = curlabelimg.at<int>(r,c);
			if(curlabel == 0) continue;
			if(curmasks.find(curlabel) == curmasks.end())
			{	
				curmasks[curlabel].create(curlabelimg.rows, curlabelimg.cols, CV_8U);
				curmasks[curlabel].setTo(0);
			}
			curmasks[curlabel].at<uchar>(r, c) = 1;
		}

		for(map<int, Mat>::iterator p1=curmasks.begin(); p1!=curmasks.end(); p1++)
		{
			vector<BasicShape> shapes;
			shapetool.ExtractConnectedComponents(p1->second, shapes);
			for (size_t j=0; j<shapes.size(); j++)
			{
				gtmasks[imgfiles[i].filename].push_back(shapes[j].mask);
			}
		}

		cout<<"Loaded NYU depth mask "<<i<<"/"<<imgfiles.size()<<endl;
	}

	return true;
}

bool NYUDepth2DataMan::LoadGTWins(const FileInfos& imgfiles, map<string, vector<ImgWin>>& gtwins)
{
	gtwins.clear();//
	
	map<string, vector<Mat>> gtmasks;
	LoadGTMasks(imgfiles, gtmasks);

	

	return true;
}