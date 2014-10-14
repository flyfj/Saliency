#include "Segmentor3D.h"
#include "..\Shared\Learners\LibLinear\blas\blas.h"


Segmentor3D::Segmentor3D(void)
{
	DIST_TH = 0.02f;
}


bool Segmentor3D::Run(const vector<vector<FeatPoint>>& super_img) {

	int imgw = super_img[0].size();
	int imgh = super_img.size();
	labels.create(imgh, imgw, CV_32S);
	labels.setTo(-1);
	labels.at<int>(0, 0) = 0;
	int label_cnt = 1;
	map<int, set<int>> equal_labels;	// save equivalent labels
	set<float> dists;
	// two-pass labeling algorithm
	// only need to look top-left ones
	Point offsets[3] = {Point(-1, 0), Point(-1, -1), Point(0, -1)};
	for (int r=0; r<imgh; r++) {
		for(int c=0; c<imgw; c++) {
			if(labels.at<int>(r,c) == -1) {
				// look at neighborhood
				bool found_sim = false;
				for(int i=0; i<3; i++) {
					int newx = MIN(MAX(0, c+offsets[i].x), imgw-1);
					int newy = MIN(MAX(0, r+offsets[i].y), imgh-1);
					Point refpt(newx, newy);
					// don't compare with self
					if(refpt.x == c && refpt.y == r) continue;

					int ref_label = labels.at<int>(refpt);
					int& cur_label = labels.at<int>(r, c);
					float dist = FeatPoint::ComputeDist(super_img[r][c], super_img[refpt.y][refpt.x]);
					dists.insert(dist);
					if( dist < DIST_TH ) {
						if( cur_label == -1 ) {
							cur_label = ref_label;
						}
						else {
							if(ref_label != cur_label) { 
								equal_labels[MIN(cur_label, ref_label)].insert(MAX(cur_label, ref_label));
							}
						}
						found_sim = true;
					}
				}
				// no similar neighbors, give new label
				if(!found_sim) labels.at<int>(r, c) = label_cnt++;
			}
		}
	}

	// aggregate equivalent labels
	int new_label_cnt = 0;
	vector<int> new_labels(label_cnt, -1);
	for(auto pi=equal_labels.begin(); pi!=equal_labels.end(); pi++) {
		cout<<pi->first<<endl;
		if(new_labels[pi->first] == -1) 
			new_labels[pi->first] = new_label_cnt++;
		for(auto val : pi->second) new_labels[val] = new_labels[pi->first];
	}
	for(int& lval : new_labels) {
		if(lval == -1) lval = new_label_cnt++;
	}
	cout<<"total labels: "<<new_label_cnt;

	// update label map
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		labels.at<int>(r, c) = new_labels[labels.at<int>(r, c)];
	}

	// draw
	vector<Vec3b> seg_colors(new_label_cnt);
	for(size_t i=0; i<seg_colors.size(); i++) 
		seg_colors[i] = Vec3b((uchar)rand()%255, (uchar)rand()%255, (uchar)rand()%255);
	Mat seg_img(labels.rows, labels.cols, CV_8UC3);
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		seg_img.at<Vec3b>(r, c) = seg_colors[labels.at<int>(r,c)];
	}
	imshow("segimg", seg_img);
	cv::waitKey(0);

	return true;
}

bool Segmentor3D::RunRegionGrowing(const Mat& pts3d_bmap) {

	Mat bmap_th;
	threshold(pts3d_bmap, bmap_th, DIST_TH, 255, CV_THRESH_BINARY_INV);
	imshow("", bmap_th);
	waitKey(10);
	labels.create(pts3d_bmap.rows, pts3d_bmap.cols, CV_32S);
	labels.setTo(-1);
	int label_cnt = 0;
	for(int r=0; r<pts3d_bmap.rows; r++) for(int c=0; c<pts3d_bmap.cols; c++) {
		if(labels.at<int>(r, c) == -1 && bmap_th.at<float>(r,c) > 0) {
			Mat cur_mask(pts3d_bmap.rows+2, pts3d_bmap.cols+2, CV_8U);
			cur_mask.setTo(0);
			Mat bmap = bmap_th.clone();
			Rect valid_roi = Rect(1, 1, cur_mask.cols-2, cur_mask.rows-2);
			int flags = 8 + (255 << 8) + CV_FLOODFILL_MASK_ONLY;
			Rect ccomp;
			int area = floodFill(bmap, cur_mask, Point(c, r), 255, &ccomp, Scalar(DIST_TH, DIST_TH, DIST_TH), Scalar(DIST_TH, DIST_TH, DIST_TH), flags);
			labels.setTo(label_cnt++, cur_mask(valid_roi));
		}
	}
	cout<<label_cnt<<endl;

	// draw
	cv::RNG rng_gen(NULL);
	vector<Vec3b> seg_colors(label_cnt);
	for(size_t i=0; i<seg_colors.size(); i++)
		seg_colors[i] = Vec3b((uchar)rng_gen.uniform(0, 255), (uchar)rng_gen.uniform(0, 255), (uchar)rng_gen.uniform(0, 255));
	Mat seg_img(labels.rows, labels.cols, CV_8UC3);
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		int label = labels.at<int>(r,c);
		seg_img.at<Vec3b>(r, c) = (label == -1? Vec3b(0,0,0):seg_colors[label]);
	}
	imshow("segimg", seg_img);
	cv::waitKey(0);

	return true;
}

//////////////////////////////////////////////////////////////////////////

// use up too much memory
void Segmentor3D::BFS(int x, int y, int label, const vector<vector<FeatPoint>>& super_img) {
	int imgw = super_img[0].size();
	int imgh = super_img.size();
	Point offsets[4] = {Point(-1, 0), Point(0, -1), Point(1, 0), Point(0, 1)};

	queue<Point> to_visit_pts;
	to_visit_pts.push(Point(x, y));
	while(!to_visit_pts.empty()) {	
		Point cur_pt = to_visit_pts.front();
		to_visit_pts.pop();
		labels.at<int>(cur_pt) = label;
		bool found_new = false;
		for(int i=0; i<4; i++) {
			int newx = MIN(MAX(0, cur_pt.x+offsets[i].x), imgw-1);
			int newy = MIN(MAX(0, cur_pt.y+offsets[i].y), imgh-1);
			if(newx == cur_pt.x && newy == cur_pt.y) continue;
			if(labels.at<int>(newy, newx) == -1) {
				float dist = FeatPoint::ComputeDist(super_img[cur_pt.y][cur_pt.x], super_img[newy][newx]);
				if(dist < DIST_TH) {
					to_visit_pts.push(Point(newx, newy));
					found_new = true;
				}
			}
		}

		if(!found_new)
			cout<<to_visit_pts.size()<<endl;
	}
}

bool Segmentor3D::RunBFS(const vector<vector<FeatPoint>>& super_img) {
	int imgw = super_img[0].size();
	int imgh = super_img.size();
	labels.create(imgh, imgw, CV_32S);
	labels.setTo(-1);
	int label_cnt = 0;

	for (int r=0; r<imgh; r++) for(int c=0; c<imgw; c++) {
		if(labels.at<int>(r, c) == -1) BFS(c, r, label_cnt++, super_img);
	}

	// draw
	vector<Vec3b> seg_colors(label_cnt);
	for(size_t i=0; i<seg_colors.size(); i++) 
		seg_colors[i] = Vec3b((uchar)rand()%255, (uchar)rand()%255, (uchar)rand()%255);
	Mat seg_img(labels.rows, labels.cols, CV_8UC3);
	for(int r=0; r<labels.rows; r++) for(int c=0; c<labels.cols; c++) {
		seg_img.at<Vec3b>(r, c) = seg_colors[labels.at<int>(r,c)];
	}
	imshow("segimg", seg_img);
	cv::waitKey(0);

	return true;
}