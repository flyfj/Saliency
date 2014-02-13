/* To segment a static object on a table or known surface */
#include "segmentationLayer.h"
#include <stdio.h>
#include <algorithm>
#include "cv.h"

int main(int argc, char* argv[]){
	if (argc != 4){
		fprintf(stderr, "usage: %s <image> <flowMap> <tableTopImg>",argv[0]);
		exit(1);
	}

	class segLayer frame1; 
  	frame1.readImage(argv[1]);

	// Edge detection!
  	frame1.edgeCGTG();

	//Biasing!
	frame1.readFlow_flo(argv[2]);
	//frame1.readDisparity(argv[2]); // select this if using disparity map	
	frame1.learnTableSurface(argv[3],16,16,16);
	frame1.generatePbBoundary();
	frame1.displayPbBoundary(-1);

	frame1.allocateMemForContours(); // Don't forget to allocate memory to store the region contours.

	//select fixation point!
	int num=2;
	frame1.selectFixPt_interactive(num);

	//segment
	frame1.segmentAllFixs();

	//display!
	frame1.displaySegs(-1);

	//release memory!
	frame1.deallocateMemForContours();
  	return 0;
	
}
