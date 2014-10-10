#include "Feature3D.h"


Feature3D::Feature3D(void)
{
}

void Feature3D::ComputeNormals(SuperPixel& sp) {

}

void Feature3D::ComputeBoundaryMaps(const Mat& cimg, const Mat& dmap) {

	// feature channels
	// lab color boundary
	Mat lab_img;
	cvtColor(cimg, lab_img, CV_BGR2Lab);

	// 3d coordinate boundary
	RGBDTools rgbd;
	Mat pts3d;
	rgbd.KinectDepthTo3D(dmap, pts3d);

	// 3d normal boundary
	Mat normals;
	PCLManager pcl_man;
	PointCloudPoints::Ptr pcl_pts(new PointCloudPoints);
	pcl_pts->width = pts3d.cols;
	pcl_pts->height = pts3d.rows;
	pcl_pts->points.clear();
	for(int r=0; r<pts3d.rows; r++) for(int c=0; c<pts3d.cols; c++) {
		Vec3f cvpt = pts3d.at<Vec3f>(r,c);
		pcl::PointXYZ curpt(cvpt.val[0], cvpt.val[1], cvpt.val[2]);
		pcl_pts->points.push_back(curpt);
	}
	PointCloudNormals::Ptr pcl_normals(new PointCloudNormals);
	pcl_man.EstimateNormals(pcl_pts, PCL_NORMAL_INTEGRAL, pcl_normals);
	normals.create(pts3d.rows, pts3d.cols, CV_32FC3);
	for(int r=0; r<pcl_normals->height; r++) for(int c=0; c<pcl_normals->width; c++) {
		pcl::Normal curpt = pcl_normals->points[r*pcl_normals->width+c];
		normals.at<Vec3f>(r, c) = Vec3f(curpt.normal_x, curpt.normal_y, curpt.normal_z);
		//cout<<curpt.normal_x<<" "<<curpt.normal_y<<" "<<curpt.normal_z<<endl;
	}


	// compute boundary map
	Mat color_bmap(cimg.rows, cimg.cols, CV_32F);
	color_bmap.setTo(0);
	Mat pts3d_bmap = color_bmap.clone();
	Mat normal_bmap = color_bmap.clone();

	// 3X3 neighbors
	Point offsets[8] = {Point(-1, -1), Point(0, -1), Point(1, -1), Point(-1, 0), 
		Point(1, 0), Point(-1, 1), Point(0, 1), Point(1, 1)};
	for (int r=0; r<pts3d.rows; r++) {
		for(int c=0; c<pts3d.cols; c++) {
			float max_cdist = 0, max_pdist = 0, max_ndist = 0;
			for(int i=0; i<8; i++) {
				Point newpt; 
				newpt.x = MIN(MAX(c+offsets[i].x, 0), pts3d.cols-1);
				newpt.y = MIN(MAX(r+offsets[i].y, 0), pts3d.rows-1);

				float color_dist = norm(lab_img.at<Vec3b>(r, c), lab_img.at<Vec3b>(newpt), NORM_L2);
				max_cdist = MAX(max_cdist, color_dist);
				float pts_dist = norm(pts3d.at<Vec3f>(r, c), pts3d.at<Vec3f>(newpt), NORM_L2);
				max_pdist = MAX(max_pdist, pts_dist);
				float normal_dist = norm(normals.at<Vec3f>(r, c), normals.at<Vec3f>(newpt), NORM_L2);
				max_ndist = MAX(max_ndist, normal_dist);
			}
			color_bmap.at<float>(r, c) = max_cdist;
			pts3d_bmap.at<float>(r, c) = max_pdist;
			normal_bmap.at<float>(r, c) = max_ndist;
		}
	}

	// combine boundary map
	Mat t_bmap = (color_bmap + pts3d_bmap + normal_bmap) / 3;

	// show inputs
	imshow("color", cimg);
	ImgVisualizer::DrawFloatImg("dmap", dmap, Mat());
	ImgVisualizer::DrawFloatImg("color_bmap", color_bmap, Mat());
	ImgVisualizer::DrawFloatImg("pts_bmap", pts3d_bmap, Mat());
	ImgVisualizer::DrawFloatImg("normal_bmap", normal_bmap, Mat());
	ImgVisualizer::DrawFloatImg("total_bmap", t_bmap, Mat());
	pcl_man.VisualizePCL(pcl_pts, pcl_normals);
	waitKey(10);

}
