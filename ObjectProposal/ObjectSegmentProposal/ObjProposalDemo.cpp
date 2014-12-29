#include "ObjProposalDemo.h"


ObjProposalDemo::ObjProposalDemo()
{
	frameid = 0;
	DATADIR = "F:\\KinectVideos\\";
}

//////////////////////////////////////////////////////////////////////////

bool ObjProposalDemo::RunVideoDemo(SensorType stype, DemoType dtype)
{
	bool tosave = true;
	visualsearch::io::camera::OpenCVCameraIO cam;
	if(stype == SENSOR_CAMERA)
	{
		if( !cam.InitCamera() )
			return false;
	}
	KinectDataMan kinect;
	if (stype == SENSOR_KINECT)
	{
		if (!kinect.InitKinect())
			return false;
	}

	char str[100];
	frameid = 0;

	while(1)
	{
		Mat cimg, dmap;
		if(stype == SENSOR_CAMERA) {
			if( !cam.QueryNextFrame(visualsearch::io::camera::STREAM_COLOR, cimg) )
				continue;
		}
		if (stype == SENSOR_KINECT) {
			if (!kinect.GetColorDepth(cimg, dmap))
				continue;
		}

		frameid++;

		// down-sample cimg to have same size as dmap
		Size newsz;
		visualsearch::common::tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
		//resize(cimg, cimg, newsz);
		// show input
		imshow("color", cimg);
		if (!dmap.empty()) {
			//resize(dmap, dmap, newsz);
			//dmap.convertTo(dmap, CV_32F);
			imshow("depth", dmap);
			//ImgVisualizer::DrawFloatImg("depth", dmap);
		}

		if(tosave && frameid % 5 == 0) {
			// no need to save every frame
			sprintf_s(str, "frame_%d.jpg", frameid);
			imwrite(DATADIR+string(str), cimg);
			sprintf_s(str, "frame_%d_d.png", frameid);
			imwrite(DATADIR+string(str), dmap);
		}

		/*if(dtype == DEMO_OBJECT_WIN)
			RunObjWinProposal(cimg, dmap);*/
		if(dtype == DEMO_OBJECT_SEG)
			RunObjSegProposal("", cimg, dmap, Mat());
		if(dtype == DEMO_SAL)
			RunSaliency(cimg, dmap, visualsearch::processors::attention::SAL_HC);

		if( waitKey(10) == 'q' )
			break;
	}

	return true;
}

bool ObjProposalDemo::RunObjSegProposal(string fn, Mat& cimg, Mat& dmap, Mat& oimg)
{
	// propose
	vector<VisualObject> sps;
	seg_proposal.Run(cimg, dmap, 5, sps);

	/*
		save results
		1) box position with score
		2) box overlay on image and cropped image patch
		3) shape overlay on image
		4) point cloud
	*/
	char str[100];
	// box positions
	string box_fn = fn + "_box.txt";
	ofstream out(box_fn);
	vector<ImgWin> boxes;
	for (size_t i = 0; i < sps.size(); i++)
	{
		boxes.push_back(sps[i].visual_data.bbox);
		
		Mat tmp_img;
		vector<VisualObject> tmp_sps;
		tmp_sps.push_back(sps[i]);
		ImgVisualizer::DrawShapes(cimg, tmp_sps, tmp_img, true);
		sprintf_s(str, "_res_%d.png", i);
		imwrite(fn + str, tmp_img);
		tmp_img = cimg(sps[i].visual_data.bbox).clone();
		sprintf_s(str, "_box_%d.png", i);
		imwrite(fn + str, tmp_img);

		out << sps[i].visual_data.bbox.x << " " << sps[i].visual_data.bbox.y << " " <<
			sps[i].visual_data.bbox.width << " " << sps[i].visual_data.bbox.height << " " << sps[i].visual_data.scores[0] << endl;
	}
	// box overlay on image
	ImgVisualizer::DrawWinsOnImg("boxes", cimg, boxes, oimg);
	imwrite(fn + "_box.png", oimg);
	
	/*imshow("mask", sps[0].visual_data.mask * 255);
	cout << "contour length: " << sps[0].visual_data.original_contour.size() << endl;
	cout << sps[0].visual_data.area*1.0f / (sps[0].visual_data.mask.rows*sps[0].visual_data.mask.cols) << endl;
	Mat contour_pts = Mat::zeros(cimg.rows, cimg.cols, CV_8U);
	for (auto p : sps[0].visual_data.original_contour) {
	contour_pts.at<uchar>(p) = 255;
	}
	imshow("contour pts", contour_pts);*/
	
	// shape overlay on image
	cout << ImgVisualizer::DrawShapes(cimg, sps, oimg, true) << endl;
	imwrite(fn + "_shape.png", oimg);

	return true;

	// convert to 3d point cloud and output to file
	visualsearch::features::Feature3D feat3d;
	Mat pts3d;
	feat3d.ComputeKinect3DMap(dmap, pts3d);
	for (size_t i = 0; i < sps.size(); i++) {
		sprintf_s(str, "_%d.obj", i);
		ofstream out(fn + str);
		for (int r = 0; r < pts3d.rows; r++) {
			for (int c = 0; c < pts3d.cols; c++) {
				if (sps[i].visual_data.mask.at<uchar>(r, c) > 0) {
					Vec3f curval = pts3d.at<Vec3f>(r, c);
					out << "v " << curval.val[0] << " " << curval.val[1] << " " << curval.val[2] << std::endl;
				}
			}
		}
	}

	//ImgVisualizer::DrawWinsOnImg("objects", cimg, boxes, oimg);
	//resize(oimg, oimg, Size(oimg.cols*2, oimg.rows*2));
	/*char str[30];
	sprintf_s(str, "%d_0.jpg", frameid);
	imwrite(DATADIR + string(str), cimg);
	imgvis.DrawWinsOnImg("input", cimg, drawwins, oimg);
	sprintf_s(str, "%d_1.png", frameid);
	dmap.copyTo(oimg);
	imwrite(DATADIR + string(str), oimg);
	imgvis.DrawCroppedWins("obj", cimg, drawwins, 5, oimg);
	sprintf_s(str, "%d_2.jpg", frameid);
	imwrite(DATADIR + string(str), oimg);*/
	
	// check lock file
	/*string lockfn = DATADIR + "demo.lock";
	ifstream in(lockfn);
	if( in.is_open() )
		return true;*/

	// save object
	//for (size_t i=0; i<drawwins.size(); i++)
	//{
	//	// color
	//	sprintf_s(str, "obj_%d_%d_c.png", frameid, i);
	//	imwrite(DATADIR+string(str), cimg(drawwins[i]));
	//	if( dmap.empty() )
	//		continue;
	//	// depth
	//	sprintf_s(str, "obj_%d_%d_d.png", frameid, i);
	//	imwrite(DATADIR+string(str), dmap(drawwins[i]));
	//}

	// write lock file
	/*ofstream out(lockfn);
	out.close();*/

	return true;
}

//bool ObjProposalDemo::RunObjWinProposal(Mat& cimg, Mat& dmap)
//{
//	// resize image
//	Size newsz;
//	tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
//	resize(cimg, cimg, newsz);
//
//	/*Mat salmap;
//	salcomputer.ComputeSaliencyMap(cimg, SAL_HC, salmap);
//	salmap.convertTo(salmap, CV_8U, 255);
//	cvtColor(salmap, salmap, CV_GRAY2BGR);
//	imshow("salcolor", salmap);
//	waitKey(10);
//	*/
//
//	// get objects
//	vector<ImgWin> objwins, salwins;
//	if( !bing.GetProposals(cimg, objwins, 1000) )
//		return false;
//
//	// rank
//	vector<int> sorted_ids;
//	ranker.RankWindowsBySaliency(cimg, objwins, sorted_ids);
//
//	// nms
//	vector<ImgWin> drawwins = visualsearch::processors::nms(objwins, 0.4f);
//	drawwins.resize(MIN(drawwins.size(), 10));
//
//	//for (size_t i=0; i<MIN(sorted_ids.size(), 10); i++)
//		//drawwins.push_back(objwins[sorted_ids[i]]);
//	//objwins.erase(objwins.begin()+10, objwins.end());
//	
//	Mat oimg;
//	char str[30];
//	imgvis.DrawWinsOnImg("input", cimg, drawwins);
//	sprintf_s(str, "e:\\res\\objectness\\%d_1.jpg", frameid);
//	imwrite(str, cimg);
//	imgvis.DrawCroppedWins("obj", cimg, drawwins, 5, oimg);
//	sprintf_s(str, "e:\\res\\objectness\\%d_2.jpg", frameid);
//	imwrite(str, oimg);
//
//	return true;
//}

bool ObjProposalDemo::RunSaliency(Mat& cimg, Mat& dmap, visualsearch::processors::attention::SaliencyType saltype)
{
	// resize image
	Size newsz;
	tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
	resize(cimg, cimg, newsz);

	Mat salmap;
	salcomputer.ComputeSaliencyMap(cimg, saltype, salmap);

	visualsearch::processors::segmentation::ImageSegmentor segmentor;
	segmentor.m_dThresholdK = 30.f;
	segmentor.seg_type_ = visualsearch::processors::segmentation::OVER_SEG_GRAPH;
	cout << "seg num " << segmentor.DoSegmentation(cimg) << endl;
	Mat sp_sal_map = Mat::zeros(cimg.rows, cimg.cols, CV_32F);
	for (auto& p : segmentor.superPixels) {
		sp_sal_map.setTo(mean(salmap, p.visual_data.mask).val[0], p.visual_data.mask);
	}
	normalize(sp_sal_map, sp_sal_map, 1, 0, NORM_MINMAX);

	imshow("color", cimg);
	imshow("sal", sp_sal_map);

	return true;
}