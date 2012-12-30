//////////////////////////////////////////////////////////////////////////


#include "ImageSegmentor.h"
#include "GraphSegmentor/image.h"
#include "GraphSegmentor/misc.h"
#include "GraphSegmentor/pnmfile.h"
#include "GraphSegmentor/segment-image.h"

ImageSegmentor::ImageSegmentor(void)
{
	m_dSmoothSigma = 0.5f;
	m_dThresholdK = 100.f;
	m_dMinArea = 100;
}

ImageSegmentor::~ImageSegmentor(void)
{
}


void ImageSegmentor::DoSegmentation(const Mat& img)
{
	//do segmentation
	//create input image
	int imgHeight = img.rows;
	int imgWidth = img.cols;
	image<rgb> *input = new image<rgb>(imgWidth, imgHeight);
	rgb val;
	for(int y=0; y<imgHeight; y++)
	{
		for(int x=0; x<imgWidth; x++)
		{
			val.b = img.at<Vec3b>(y, x)[0];
			val.g = img.at<Vec3b>(y, x)[1];
			val.r = img.at<Vec3b>(y, x)[2];
			imRef(input, x, y) = val;
		}
	}

	//segment
	int num_ccs = 0;
	image<int> *index = new image<int>(imgWidth, imgHeight);
	image<rgb> *seg = segment_image(input, m_dSmoothSigma, m_dThresholdK, m_dMinArea, &num_ccs, index);
	
	// set index image
	m_idxImg.release();
	m_idxImg.create(imgHeight, imgWidth, CV_32S);
	for(int y=0; y<imgHeight; y++)
	{
		for(int x=0; x<imgWidth; x++)
		{
			int id = imRef(index, x, y);
			m_idxImg.at<int>(y, x) = id;
		}
	}
	// set color segment image
	m_segImg.release();
	m_segImg.create(img.size(), CV_8UC3);
	for(int y=0; y<imgHeight; y++)
	{
		for(int x=0; x<imgWidth; x++)
		{
			val = imRef(seg, x, y);
			m_segImg.at<Vec3b>(y, x)[0] = val.b;
			m_segImg.at<Vec3b>(y, x)[1] = val.g;
			m_segImg.at<Vec3b>(y, x)[2] = val.r;
		}
	}
	//show segment image
	imshow("Segment", m_segImg);
	waitKey(10);
	printf("Segmentation is done. Total component: %d.\n", num_ccs);

	//extract components
	superpixels.clear();
	superpixels.reserve(num_ccs);
	vector<Superpixel> temp_sps;
	for(int i=0; i<num_ccs; i++)
	{
		Superpixel segt;
		segt.centroid = Point(0,0);
		segt.bbox = Rect(-1,-1,0,0);
		segt.area = 0;
		//create mask image
		segt.sp_mask.create(img.size(), CV_8U);
		segt.sp_mask.setTo(0);
		temp_sps.push_back(segt);
	}

	//	set masks
	for(int y=0; y<imgHeight; y++)
	{
		for(int x=0; x<imgWidth; x++)
		{
			int id = imRef(index, x, y);	//segment id
			temp_sps[id].sp_mask.at<uchar>(y, x) = 255;
		}
	}

	// compute features
	Mat cur_mask(img.size(), CV_8U);
	for(size_t i=0; i<temp_sps.size(); i++)
	{
		temp_sps[i].sp_mask.copyTo(cur_mask);	// to be modified
		vector<vector<Point>> contours;
		findContours(cur_mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		if(contours.empty())	// non-valid superpixel
			continue;
		temp_sps[i].bbox = boundingRect( Mat(contours[0]) );
		if(temp_sps[i].bbox.x == 0 || temp_sps[i].bbox.y == 0 || \
			temp_sps[i].bbox.x + temp_sps[i].bbox.width >= img.cols || \
			temp_sps[i].bbox.y + temp_sps[i].bbox.height >= img.rows)
			temp_sps[i].ifBoundarySeg = true;
		temp_sps[i].centroid = Point2f(temp_sps[i].bbox.x+temp_sps[i].bbox.width/2, temp_sps[i].bbox.y+temp_sps[i].bbox.height/2);
		temp_sps[i].area = countNonZero(temp_sps[i].sp_mask);

		// add to real results
		superpixels.push_back( temp_sps[i] );
	}

	// release
	delete index;
	delete seg;
	delete input;

}