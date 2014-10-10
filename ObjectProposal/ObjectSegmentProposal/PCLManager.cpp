#include "PCLManager.h"


PCLManager::PCLManager(void)
{
	model_ss = 0.12f;
	nn_rad = 0.03;
	nn_k = 20;
	rf_rad = 0.08f;
	desc_rad = 0.08f;
	desc_k = 15;

}

//////////////////////////////////////////////////////////////////////////

bool PCLManager::WritePCD()
{
	pcl::PointCloud<pcl::PointXYZ> cloud;
	
	// Fill in the cloud data
	cloud.width    = 5;
	cloud.height   = 1;
	cloud.is_dense = false;
	cloud.points.resize (cloud.width * cloud.height);

	for (size_t i = 0; i < cloud.points.size (); ++i)
	{
		cloud.points[i].x = 1024 * rand () / (RAND_MAX + 1.0f);
		cloud.points[i].y = 1024 * rand () / (RAND_MAX + 1.0f);
		cloud.points[i].z = 1024 * rand () / (RAND_MAX + 1.0f);
	}

	pcl::io::savePCDFileASCII ("test_pcd.pcd", cloud);
	std::cerr << "Saved " << cloud.points.size () << " data points to test_pcd.pcd." << std::endl;

	for (size_t i = 0; i < cloud.points.size (); ++i)
		std::cerr << "    " << cloud.points[i].x << " " << cloud.points[i].y << " " << cloud.points[i].z << std::endl;

	return true;
}

bool PCLManager::PreprocessPCL(PointCloudPoints::Ptr& input)
{
	PointCloudPoints::Ptr output(new PointCloudPoints);

	// filter noise using statistical outlier removal
	pcl::StatisticalOutlierRemoval<PointType> sor;
	sor.setInputCloud(input);
	sor.setMeanK(50);
	sor.setStddevMulThresh(1);
	sor.filter(*output);

	// normalize scale to unit
	NormalizePCLScale(output);

	// copy back
	input.reset();
	input = output;

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool PCLManager::Detect3DKeypoints(const PointCloudPoints::ConstPtr input, const PCLOptions option, PointCloudPoints::Ptr output)
{
	output->clear();

	if(option == PCL_KEYPOINT_UNIFORM)
	{
		pcl::UniformSampling<PointType> uniform_sampling;
		pcl::PointCloud<int> sampled_indices;
		uniform_sampling.setInputCloud (input);
		//uniform_sampling.setKSearch(5);
		uniform_sampling.setRadiusSearch (model_ss);
		uniform_sampling.compute (sampled_indices);
		pcl::copyPointCloud (*input, sampled_indices.points, *output);
		
		std::cout << "Total points: " << input->size () << "; Selected Keypoints: " << output->size () << std::endl;
	}
	if(option == PCL_KEYPOINT_VOXELGRID)
	{
		// Create the filtering object
		pcl::VoxelGrid<PointType> vg;
		vg.setInputCloud (input);
		vg.setLeafSize (0.01f, 0.01f, 0.01f);
		vg.filter (*output);
	}
	
	
	return true;
}

bool PCLManager::EstimateNormals(const PointCloudPoints::Ptr input, PCLOptions option, PointCloudNormals::Ptr normals)
{
	// estimate normals
	normals->clear();
	
	if(option == PCL_NORMAL_REGULAR)
	{
		// Create the normal estimation class, and pass the input dataset to it
		pcl::NormalEstimation<PointType, NormalType> ne;
		ne.setInputCloud (input);

		// Create an empty kdtree representation, and pass it to the normal estimation object.
		// Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
		pcl::search::KdTree<PointType>::Ptr tree (new pcl::search::KdTree<PointType> ());
		ne.setSearchMethod (tree);

		// Use all neighbors in a sphere of radius xcm
		//ne.setRadiusSearch (nn_rad);
		ne.setKSearch(nn_k);

		// Compute the features
 		ne.compute (*normals);

	}
	if(option == PCL_NORMAL_INTEGRAL)
	{
		pcl::IntegralImageNormalEstimation<PointType, NormalType> ne;
		ne.setNormalEstimationMethod (ne.COVARIANCE_MATRIX);
		ne.setMaxDepthChangeFactor(0.02f);
		ne.setNormalSmoothingSize(10.0f);
		ne.setInputCloud(input);
		ne.compute(*normals);
	}

	// check normal validity
	float valid_num = 0;
	for (int i = 0; i < normals->size(); i++)
	{
		if (!pcl::isFinite<pcl::Normal>(normals->at(i)))
		{
			//PCL_WARN("normals[%d] is not finite\n", i);
		}
		else
			valid_num++;
	}
	cout<<"Valid(finite) normal percentage: "<<valid_num / normals->size() * 100<<"%"<<endl;

	return true;
}

//////////////////////////////////////////////////////////////////////////

void PCLManager::NormalizePCLScale(const PointCloudPoints::Ptr cloud)
{
	// centralize
	PointType centroid;
	centroid.x = centroid.y = centroid.z = 0;
	for(PointType pt : cloud->points)
	{
		centroid.x += pt.x;
		centroid.y += pt.y;
		centroid.z += pt.z;
	}
	centroid.x /= cloud->size();
	centroid.y /= cloud->size();
	centroid.z /= cloud->size();

	for(PointType& pt : cloud->points)
	{
		pt.x -= centroid.x;
		pt.y -= centroid.y;
		pt.z -= centroid.z;
	}

	// find point with maximum norm
	float maxnorm = 0;
	for (PointType pt : cloud->points)
	{
		float curnorm = sqrt(pt.x*pt.x + pt.y*pt.y + pt.z*pt.z);
		if(curnorm > maxnorm) maxnorm = curnorm;
	}

	// normalize
	for (PointType& pt : cloud->points)
	{
		pt.x /= maxnorm;
		pt.y /= maxnorm;
		pt.z /= maxnorm;
	}

}

bool PCLManager::GetPCLRange(const PointCloudPoints::Ptr input, pcl::PointXYZ& minp, pcl::PointXYZ& maxp)
{
	minp = maxp = input->points[0];
	for (int i=0; i<input->points.size(); i++)
	{
		minp.x = MIN(input->points[i].x, minp.x);
		minp.y = MIN(input->points[i].y, minp.y);
		minp.z = MIN(input->points[i].z, minp.z);
		maxp.x = MAX(input->points[i].x, maxp.x);
		maxp.y = MAX(input->points[i].y, maxp.y);
		maxp.z = MAX(input->points[i].z, maxp.z);
	}

	printf("Minp: %f, %f, %f; Maxp: %f, %f, %f\n", minp.x, minp.y, minp.z, maxp.x, maxp.y, maxp.z);

	return true;
}

bool PCLManager::ComputePointDescriptors(const PointCloudPoints::Ptr input, const PointCloudPoints::Ptr keypoints, 
											 const FeatureType3D option, PointCloudDescriptors::Ptr feats)
{
	feats->clear();

	// extract features for keypoints
	if(option == FEAT_3D_SHOT)
	{
		// compute normal for all points
		PointCloudNormals::Ptr normals (new PointCloudNormals);
		EstimateNormals(input, PCL_NORMAL_REGULAR, normals);

		// compute descriptors
		pcl::SHOTEstimationOMP<PointType, NormalType> descr_est;
		//descr_est.setKSearch(desc_k);
		descr_est.setRadiusSearch (desc_rad);

		descr_est.setInputCloud (keypoints);
		descr_est.setInputNormals (normals);
		descr_est.setSearchSurface (input);
		descr_est.compute (*feats);
	}
	if(option == FEAT_3D_FPFH)
	{
		/*feats.reset();
		feats = PointCloudFPFH33::Ptr(new PointCloudFPFH33);
		pcl::FPFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::FPFHSignature33> fpfh_est;
		fpfh_est.setInputCloud (input);
		fpfh_est.setInputNormals (normals);
		fpfh_est.setSearchMethod (pcl::search::KdTree<pcl::PointXYZ>::Ptr(new pcl::search::KdTree<pcl::PointXYZ>()));
		fpfh_est.setRadiusSearch (0.05f);
		fpfh_est.compute (*feats);*/
	}
	if(option == FEAT_3D_ESF)
	{
		/*pcl::ESFEstimation<PointType, PointDescriptorType> dest_est;
		dest_est.setInputCloud(input);
		dest_est.compute(*feats);*/
	}
	

	return true;
}

bool PCLManager::SerializeDescriptors(const string& fn, const PointCloudDescriptors::Ptr descs, const FeatureType3D desc_type)
{
	ofstream out(fn, ios::binary);
	// number of descriptors
	int count = descs->size();
	out.write((char*)&count, sizeof(int));
	if(desc_type == FEAT_3D_SHOT)
	{
		// dim
		int dim = 352+9;
		out.write((char*)&dim, sizeof(int));
		// samples
		for(int i=0; i<descs->size(); i++)
		{
			// rf
			for(int k=0; k<9; k++)
				out.write((char*)&descs->at(i).rf[k], sizeof(float));
			// shot
			for(int k=0; k<352; k++)
				out.write((char*)&descs->at(i).descriptor[k], sizeof(float));
		}
	}
	if(desc_type == FEAT_3D_ESF)
	{
		int dim = descs->points[0].descriptorSize();
		out.write((char*)&dim, sizeof(int));
		//for (int i=0; i<dim; i++)
			//out.write((char*)&descs->points[0].histogram[i], sizeof(float));
	}

	cout<<"Data saved to file: "<<fn<<endl;

	return true;
}

bool PCLManager::DeserializeDescriptors(const string& fn, PointCloudDescriptors::Ptr descs, const FeatureType3D desc_type)
{
	descs->clear();

	ifstream in(fn, ios::binary);
	// count
	int count = 0;
	in.read((char*)&count, sizeof(int));
	if(desc_type == FEAT_3D_SHOT)
	{
		// dim
		int dim = 0;
		in.read((char*)&dim, sizeof(int));
		vector<float> x(count * dim);
		in.read((char*)&x[0], sizeof(float)*count*352);

		descs->resize(count);
		for(int i=0; i<count; i++)
		{
			for(int j=0; j<9; j++)
			descs->at(i).rf[j] = x[i*dim+j];
			for(int j=0; j<352; j++)
			descs->at(i).descriptor[j] = x[i*dim+9+j];
		}
	}
	if(desc_type == FEAT_3D_ESF)
	{
		descs->resize(count);
		int dim = 0;
		in.read((char*)&dim, sizeof(int));
		//for(int i=0; i<dim; i++)
		//	in.read((char*)&descs->points[0].histogram[i], sizeof(float));
	}


	return true;
}

//////////////////////////////////////////////////////////////////////////

bool PCLManager::VisualizePCL(const PointCloudPoints::Ptr input, const PointCloudNormals::Ptr normals)
{	
	pcl::visualization::PCLVisualizer viewer ("PCL Viewer");
	viewer.setBackgroundColor (0.0, 0.0, 0.5);
	if(normals != NULL && normals->points.size() == input->points.size()) {
		pcl::visualization::PointCloudColorHandlerCustom<PointType> input_color_handler(input, 255, 0, 0);
		viewer.addPointCloud(input, input_color_handler, "input");
		viewer.addPointCloudNormals<PointType, NormalType>(input, normals, 200, 0.03, "normals");
	}
	else
		viewer.addPointCloud (input, "in_cloud");

	while (!viewer.wasStopped ())
	{
		viewer.spinOnce ();
	}

	return true;
}

bool PCLManager::ShowKeypoints(const PointCloudPoints::Ptr input, const PointCloudPoints::Ptr keypoints)
{
	pcl::visualization::PCLVisualizer viewer("matching");
	pcl::visualization::PointCloudColorHandlerCustom<PointType> input_color_handler (input, 255, 255, 255);
	viewer.addPointCloud(input, input_color_handler, "input");

	pcl::visualization::PointCloudColorHandlerCustom<PointType> input_keypoints_color_handler (keypoints, 0, 0, 255);
	viewer.addPointCloud (keypoints, input_keypoints_color_handler, "input_keypoints");
	viewer.setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "input_keypoints");

	// show
	while (!viewer.wasStopped())
	{
		viewer.spinOnce();
	}

	viewer.close();

	return true;
}


//////////////////////////////////////////////////////////////////////////

