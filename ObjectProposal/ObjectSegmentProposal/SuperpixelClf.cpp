#include "SuperpixelClf.h"


SuperpixelClf::SuperpixelClf(void)
{
	// init forest params
	DTreeTrainingParams tparams;
	tparams.feature_num = 500;
	tparams.th_num = 80;
	tparams.min_samp_num = 50;
	tparams.MaxLevel = 9;
	
	rfparams.tree_params = tparams;
	rfparams.num_trees = 6;
	rfparams.split_disjoint = false;

	rf.Init(rfparams);

	rf_model_file = "sp_clf_slic.model";
	db_info_file = "db_info.dat";
	sp_dict_file = "sp_dict.dat";
}

bool SuperpixelClf::Init(int sp_feats) {
	sp_feats_ = sp_feats;

	// load dictionary
	//dict_gen.LoadDictionary(sp_dict_file, DICT_KMEANS, true);

	//// load db info
	//ifstream in(db_info_file);
	//if( in.is_open() ) {
	//	Point val;
	//	int num;
	//	in>>num;
	//	for(int i=0; i<num; i++) {
	//		in>>val.x>>val.y;
	//		label_map[val.x] = val.y;
	//	}
	//}
	// load classifier
	ifstream in1(rf_model_file);
	if( rf.Load(in1) ) 
		return true;
	else
		return false;
}


bool SuperpixelClf::Train(DatasetName db_name) {

	map<int, ObjectCategory> gt_objs;

	// load ground truth
	DataManagerInterface* db_man = NULL;
	if(db_name == DB_NYU2_RGBD)
		db_man = new NYUDepth2DataMan;
	else return false;

	FileInfos imgfns, dmapfns;
	db_man->GetImageList(imgfns);
	imgfns.erase(imgfns.begin()+100, imgfns.end());
	db_man->GetDepthmapList(imgfns, dmapfns);

	ImageSegmentor img_segmentor;
	img_segmentor.m_dMinArea = 100;
	img_segmentor.m_dThresholdK = 30;
	img_segmentor.slic_seg_num_ = 300;
	img_segmentor.seg_type_ = OVER_SEG_SLIC;

	int label_cnt = 0;
	label_map.clear();

	Mat samp_data;
	Mat samp_label;

	Mat eg_cimg, eg_dmap;
	Mat possamps, negsamps;
	for(size_t i=0; i<imgfns.size(); i++) {

		Mat cimg = imread(imgfns[i].filepath);
		Size newsz;
		ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
		resize(cimg, cimg, newsz);
		Mat dmap;
		db_man->LoadDepthData(dmapfns[i].filepath, dmap);
		resize(dmap, dmap, newsz);
		dmap.convertTo(dmap, CV_32F);

		FileInfos tmp_imgfns;
		tmp_imgfns.push_back(imgfns[i]);
		map<string, vector<VisualObject>> gt_masks;
		db_man->LoadGTMasks(tmp_imgfns, gt_masks);

		seg_processor.Init(cimg, dmap);

		img_segmentor.DoSegmentation(cimg);
		vector<SuperPixel>& sps = img_segmentor.superPixels;

		for(auto& cur_sp : sps) {
			seg_processor.ExtractSegmentBasicFeatures(cur_sp);
			cur_sp.centroid.x /= cimg.cols;
			cur_sp.centroid.y /= cimg.rows;

			// extract features
			Mat cur_feat;
			seg_processor.ExtractSegmentVisualFeatures(cur_sp, sp_feats_, cur_feat);
			cur_feat.copyTo(cur_sp.namedFeats["total"]);

			// find the best overlapped gt obj
			float best_overlap = 0;
			cur_sp.id = -1;
			for(size_t k=0; k<gt_masks[imgfns[i].filename].size(); k++) {

				VisualObject& cur_gt = gt_masks[imgfns[i].filename][k];
				if(cur_gt.visual_desc.mask.rows != newsz.height) resize(cur_gt.visual_desc.mask, cur_gt.visual_desc.mask, newsz);

				float ratio = countNonZero(cur_sp.mask & cur_gt.visual_desc.mask)*1.f / cur_sp.area;
				if(ratio > best_overlap) {
					best_overlap = ratio;
					cur_sp.id = k;
				}
			}
		}

		// check pairs
		Mat adj_mat;
		img_segmentor.ComputeAdjacencyMat(sps, adj_mat);
		for(size_t id1=0; id1<sps.size(); id1++) {
			for(size_t id2=id1+1; id2<sps.size(); id2++) {
				if(adj_mat.at<int>(id1, id2) == 0) continue;

				Mat cur_feat;
				hconcat(sps[id1].namedFeats["total"], sps[id2].namedFeats["total"], cur_feat);
				if(sps[id1].id == sps[id2].id) {
					possamps.push_back(cur_feat);
				}
				else {
					negsamps.push_back(cur_feat);
				}
			}
		}
			

			// add to set
		/*samp_data.push_back(cur_feat);
		if(best_gt != -1) samp_label.push_back(gt_masks[imgfns[i].filename][best_gt].category_id);
		else samp_label.push_back(0);*/
			//if(label_map.find(gt.category_id) == label_map.end()) label_map[cur_gt.category_id] = label_cnt++;
			//gt_objs[label_map[cur_gt.category_id]].objects.push_back(cur_gt);


		cout<<"finished image: "<<i<<"/"<<imgfns.size()<<endl;
	}

	delete db_man;
	db_man = NULL;

#ifdef SP_DICT
	// train a dictionary
	cout<<"total sp number: "<<samp_data.rows<<endl;
	int dict_size = 50;
	dict_gen.BuildDictionary(samp_data, dict_size, DICT_KMEANS);
	dict_gen.BuildDictionaryIndex(dict_gen.dictionary);
	dict_gen.SaveDictionary(sp_dict_file, true);

	// compute border pair statistics
	Mat scoremap(dict_size, dict_size, CV_32F);
	scoremap.setTo(0);

	for(int r=0; r<samp_data.rows; r++) for(int c=r+1; c<samp_data.rows; c++) {
		if(samp_label.at<int>(r) == samp_label.at<int>(c)) continue;
		int sp1_dict_id, sp2_dict_id;
		vector<DMatch> matches;
		dict_gen.Match2Dictionary(samp_data.row(r), matches);
		sp1_dict_id = matches[0].trainIdx;
		dict_gen.Match2Dictionary(samp_data.row(c), matches);
		sp2_dict_id = matches[0].trainIdx;
		scoremap.at<float>(sp1_dict_id, sp2_dict_id)++;
		scoremap.at<float>(sp2_dict_id, sp1_dict_id) = scoremap.at<float>(sp1_dict_id, sp2_dict_id);
	}

	resize(scoremap, scoremap, Size(scoremap.cols*3, scoremap.rows*3));
	ImgVisualizer::DrawFloatImg("border pair map", scoremap);

#endif

//#define SP_LEARN
#ifdef SP_LEARN
	// save db info
	ofstream out0(db_info_file);
	out0<<label_map.size()<<endl;
	for (auto pi : label_map) {
		out0<<pi.first<<" "<<pi.second<<endl;
	}
#endif

	// divide into train and test data
	Mat rank_train_data, rank_test_data, rank_train_label, rank_test_label;
	for (int r=0; r<possamps.rows; r++)
	{
		if(r<possamps.rows*0.7) {
			rank_train_data.push_back(possamps.row(r));
			rank_train_label.push_back(1);
		}
		else {
			rank_test_data.push_back(possamps.row(r));
			rank_test_label.push_back(1);
		}
	}
	for (int r=0; r<negsamps.rows; r++)
	{
		if(r<negsamps.rows*0.7) {
			rank_train_data.push_back(negsamps.row(r));
			rank_train_label.push_back(0);
		}
		else {
			rank_test_data.push_back(negsamps.row(r));
			rank_test_label.push_back(0);
		}
	}
	/*for (auto pi=gt_objs.begin(); pi!=gt_objs.end(); pi++) {
		for (int r=0; r<pi->second.objects.size(); r++) {
			if(r<pi->second.objects.size()*0.8) {
				rank_train_data.push_back(pi->second.objects[r].visual_desc.img_desc);
				rank_train_label.push_back(pi->first);
			}
			else {
				rank_test_data.push_back(pi->second.objects[r].visual_desc.img_desc);
				rank_test_label.push_back(pi->first);
			}
		}
	}*/

	std::cout<<"Training data ready."<<endl;

	std::cout<<"Start to train..."<<endl;
	ofstream out(rf_model_file);
	rf.Train(rank_train_data, rank_train_label);
	rf.Save(out);

	rf.EvaluateRandomForest(rank_test_data, rank_test_label, 2);


	return true;
}

bool SuperpixelClf::Predict(const Mat& cimg, const Mat& dmap) {

	seg_processor.Init(cimg, dmap);
	// show boundary map
	ImageSegmentor img_segmentor;
	img_segmentor.m_dThresholdK = 30;
	img_segmentor.m_dMinArea = 120;
	img_segmentor.slic_seg_num_ = 300;
	img_segmentor.seg_type_ = visualsearch::processors::segmentation::OVER_SEG_SLIC;
	int init_seg_num = img_segmentor.DoSegmentation(cimg);
	imshow("seg", img_segmentor.m_segImg);

	vector<SuperPixel>& sps = img_segmentor.superPixels;
	for (size_t i=0; i<sps.size(); i++) 
	{
		seg_processor.ExtractSegmentBasicFeatures(sps[i]);
	}

	Mat bmap(cimg.rows, cimg.cols, CV_32F);
	bmap.setTo(0);
	// compute pair-wise distance
	Mat adjMat;
	img_segmentor.ComputeAdjacencyMat(sps, adjMat);
	for(int r=0; r<adjMat.rows; r++) {
		for(int c=r+1; c<adjMat.rows; c++) {
			if(adjMat.at<int>(r,c) > 3) {
				Mat feat1, feat2;
				seg_processor.ExtractSegmentVisualFeatures(sps[r], sp_feats_, feat1);
				seg_processor.ExtractSegmentVisualFeatures(sps[c], sp_feats_, feat2);
				Mat total_feat;
				hconcat(feat1, feat2, total_feat);
				vector<double> scores;
				rf.Predict(total_feat, scores);
				for(size_t i=0; i<sps[r].original_contour.size(); i++)
					bmap.at<float>(sps[r].original_contour[i]) = scores[0];
			}
		}
	}

	ImgVisualizer::DrawFloatImg("bmap", bmap);

	return true;
}

bool SuperpixelClf::Predict(const Mat& samp, vector<double>& scores) {
	// extract features
	rf.Predict(samp, scores);

	return true;
}

float SuperpixelClf::LabelDistributionDist(const vector<double>& label1, const vector<double>& label2) {
	float dist = 0;
	for(size_t i=0; i<label1.size(); i++)
		dist += fabs(label1[i]-label2[i]);

	return dist;
}