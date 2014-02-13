To compile: > Use visual studios. 

1  example1.cpp is for a single image case.
2. example2.cpp is for a video. Inputs are the image frame and its optical flow map. 


EXPLANATION:
There are three important steps:
1. Generating probabilistic boundary edge map.
   	
2. Selecting a fixation point.

3. Segmenting the region containing that fixation point

"segLayer" is a class that has functions to execute the following three steps: 
In step 1, all the available cues are used to generate the probabilistic boundary edge map using generatePbBoundary().

In step 2, fixation points are selected interactively using selectFixPt_interactive(int ). The other ways to select fixation points are assignFixPt(x,y) and reading directly from a text file using readFixPts() function.

In the last step, if there is only one fixation point, you can use segmentCurrFixPt(). However,
if you have multiple fixation points and want to segment for all of them at once, use segmentAllFixs(). 

Output:
Can be binary masks. But more efficient way is to store in an xml file using saveContoursIntoXML(char*). The matlab functions are provided to read the stored closed contours from the xml file. 

CAUTION:  Don't forget to allocate memory using allocateMemForContours() to store the region contours. 

To read about the segmentation process, refer to Ajay Mishra, Yiannis Aloimonos, C.L. Fah "Active Segmentation With Fixation", in ICCV 2009. 
Also cite this paper, if you use the code in your research.

For any help, feel free to write to mishraka@umiacs.umd.edu
