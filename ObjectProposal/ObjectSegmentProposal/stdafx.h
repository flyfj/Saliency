

//////////////////////////////////////////////////////////////////////////
// preinclude headers

#pragma once

#include <visualsearch/common/Core2d.h>
#include <visualsearch/common/tools/Tools.h>
#include <visualsearch/common/tools/ImgVisualizer.h>
#include <visualsearch/common/tools/RGBDTools.h>

#include <visualsearch/io/datasets/RGBDECCV14.h>
#include <visualsearch/io/datasets/Berkeley3DDataManager.h>
#include <visualsearch/io/datasets/NYUDepth2DataMan.h>
#include <visualsearch/io/camera/OpenCVCameraIO.hpp>

#include <visualsearch/features/color/ColorDescriptors.h>
#include <visualsearch/features/3d/DepthDescriptors.h>
#include <visualsearch/features/3d/Feature3D.h>

#include <visualsearch/processors/ShapeAnalyzer.h>
#include <visualsearch/processors/segmentation/ImageSegmentor.h>
#include <visualsearch\processors\segmentation\SegmentProcessor.h>
#include <visualsearch/processors/segmentation/IterativeSegmentor.h>
#include <visualsearch/processors/nms.hpp>
#include <visualsearch/processors/attention/CenterSurroundFeatureContraster.h>
#include <visualsearch/processors/attention/composition_saliency/SalientRGBDRegionDetector.h>
#include <visualsearch/processors/attention/composition_saliency/SalientRegionDetector.h>
#include <visualsearch/processors/attention/SaliencyComputer.h>
#include <visualsearch/processors/attention/GlobalContrastSaliency.h>

#include <visualsearch/learners/trees/RandomForest.hpp>
#include <visualsearch/learners/LearnerTools.h>

#include <visualsearch/search/Searcher.h>
#include <visualsearch/search/binary_code/LSHCoder.h>

#include <Eigen/Eigen>
