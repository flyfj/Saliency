//-----------------------------------------------
// Segmentation Layer
// Author: Ajay K Mishra
// Created on July 19, 2010
// Modified on Dec 01, 2010
//-----------------------------------------------

#ifndef _SEG_LAYER
#define _SEG_LAYER

#ifndef _M_PI
#define _M_PI acos(-1.0)
#endif


#include <opencv2\opencv.hpp>
#include "edgeDetection\savgol.h"
#include "edgeDetection\sobelEdge.h"
#include "pbBoundary\pbBoundary.h"
#include "misc\misc.h"
#include "segmentation\segFixatedRegion.h"
#include <vector>
using namespace std;

class segLayer{
 protected:
  IplImage* img;
  IplImage* flowU;
  IplImage* flowV;
  IplImage* dispMap;
  IplImage* edgeGrad;
  IplImage* edgeOri;
  IplImage* pbBoundary;
  IplImage* mask;
  CvMemStorage* storage;
  double*   groundColorHist_3D;

  int width;
  int height;
  int numOri;
  int nL, na, nb;

  bool flowFLAG;
  bool surfaceFLAG;
  bool disparityFLAG;
  
  std::vector<CvPoint> fixPts;
  CvPoint currFixPt;
  
  std::vector<CvSeq *> allContours;

  public:
  //constuctor & destructors
  segLayer();
  ~segLayer();
  
  // I/O
  int readImage(char* imgFileName);
  int setImage(IplImage* imgPtr);
  int getImgWidth();
  int getImgHeight();
  int setEdgeGrad(char* );
  int setEdgeOri(char* );
  
  //Visual Cues
  int edgeCGTG(); 	/* using color and texture gradient (slowest but most reliable) */
  int edgeCG();   	/* using color gradient only */
  int edgeBG();   	/* using brightness gradient only */
  int edgeSobel(); 	/* sobel edge (fastest but least reliable) */
  int setDisparity(IplImage* );
  int readDisparity(char*);
  int readFlow(char* txtFileName);
  int readFlow_flo(char* floFileName);
  int setU(IplImage* u);
  int setV(IplImage* v);
  int learnTableSurface(IplImage *tableImg, int nL, int na, int nb);
  int learnTableSurface(char *tableImgName, int nL, int na, int nb);

  //pb
  int generatePbBoundary();
  int generatePbBoundary(IplImage* mask);

  //fixation
  int selectFixPt_interactive(int);
  int selectFixPt_auto();
  int readFixPts(char* fixTxtFile);
  int assignFixPt(int x, int y);
  int getNumFixPts();
  int getFixPt(CvPoint&, int ind);
  CvPoint getFixPt(int ind);
  
  //Memory
  int allocateMemForContours();	
  int deallocateMemForContours();

  //Segmentation
  int segmentCurrFix();
  int segmentSpecificFix(int ind);
  int segmentAllFixs();
  int generateRegionMask(IplImage* mask, int contourInd);  
  int clearAllContours();
  
  //Save outputs
  int saveContoursIntoXML(char* prefixTag);
  int saveRegions(char* prefixTag);
  int saveEdgeGrad(char* prefixTag);
  int savePbBoundary(char* prefixTag);
  int saveEdgeOri(char* prefixTag);
    
  //Display
  int displayImg(int delay);
  int displayMask(int delay);
  int displayEdge(int delay);
  int displayPbBoundary(int delay);
  int displayRegion(int ind);
  int displayFlowMag(int delay);
  int displaySegs(int delay);
  int displayCurrFixPt(int delay);
  int displayAllFixPts(int delay);
  int saveFixPtsImage(char* prefixTag);
};


#endif
