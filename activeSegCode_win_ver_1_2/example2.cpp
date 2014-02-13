/* To segment a moving object */
#include "segmentationLayer.h"
#include <stdio.h>
#include <algorithm>
#include "cv.h"

int main(int argc, char* argv[]){
	if (argc != 3){
		fprintf(stderr, "usage: %s <image_1> <flowMap/disparity>",argv[0]);
		exit(1);
	}

	class segLayer frame1; 
  	frame1.readImage(argv[1]);

	// Edge detection!
  	frame1.edgeCGTG();

	//Biasing with optic flow!
	frame1.readFlow_flo(argv[2]);
	frame1.generatePbBoundary();

	frame1.displayPbBoundary(300);

	frame1.saveEdgeGrad(argv[1]);
	frame1.saveEdgeOri(argv[1]);

	frame1.allocateMemForContours();

	//select fixation point!
	int num =2;
	frame1.selectFixPt_interactive(num);

	//segment
	frame1.segmentAllFixs();

	//To display results!
	frame1.displaySegs(3000);
	
	//To save binary masks!
	frame1.saveRegions(argv[1]);

	//To save closed contours into an XML file
	frame1.saveContoursIntoXML(argv[1]);
	
	//release memory!
	frame1.deallocateMemForContours();
  	return 0;
}
