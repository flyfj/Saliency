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
	vector<Point> equal_labels;	// save equivalent labels
	vector<float> dists;
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
					dists.push_back(dist);
					if( dist < DIST_TH ) {
						if( cur_label == -1 ) {
							cur_label = ref_label;
						}
						else {
							if(ref_label != cur_label) { 
								equal_labels.push_back( Point(MIN(cur_label, ref_label), MAX(cur_label, ref_label)) );
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

	sort(dists.begin(), dists.end());
	// aggregate equivalent labels
	sort(equal_labels.begin(), equal_labels.end(), [](Point a, Point b) { 
		return a.x < b.x;
	});
	int new_label_cnt = 0;
	vector<int> new_labels(label_cnt, -1);
	for(Point pt : equal_labels) {
		if(new_labels[pt.x] == -1) new_labels[pt.x] = new_label_cnt++;
		new_labels[pt.y] = new_labels[pt.x];
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
	waitKey(0);

	return true;
}
