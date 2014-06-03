/* To segment a region in a single image */
#pragma comment(lib, "segCoreLib64bit.lib") /*for 64 bit windows  & Visual Studio Express 2010 */


#include "segmentationLayer.h"
#include <stdio.h>
#include <algorithm>
#include <opencv2\opencv.hpp>


int main(int argc, char* argv[])
{
	if (argc != 2 && argc != 4){
		fprintf(stderr, "usage: %s <image> ",argv[0]);
		exit(1);
	}


	class segLayer frame1; 
  	frame1.readImage(argv[1]);

	if (argc == 2){
		// Edge detection!
  		frame1.edgeCGTG(); // For speed up, (frame1.edgeCG() can be used, but will also compromise quality)
	}
	else if (argc == 4){
		frame1.setEdgeGrad(argv[2]);
		frame1.setEdgeOri(argv[3]);
	}

	frame1.generatePbBoundary();
	frame1.displayPbBoundary(300);

	frame1.saveEdgeGrad(argv[1]);
	frame1.saveEdgeOri(argv[1]);
	frame1.savePbBoundary(argv[1]);
	
	frame1.allocateMemForContours();
	
	//select fixation point!
	int num = 2;
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
