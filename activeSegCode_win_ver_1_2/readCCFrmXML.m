function [closedContours, img, masks] = readCCFrmXML(xmlFileName)
% [closedContours, img, masks] = readCCFrmXML(xmlFileName)
% read Closed Contours from the xml file.
% Author: Ajay K Mishra
% Date: Sept 10, 2011.
%------------------------------------------
% reads the segmented closed contours, their interior and rgb image.
% Closed contours is a cell array where each element is a Nx2 matrix where
% N is the contour length, and x and y enteries are stored in the 1st and
% 2nd column.
% FYI: in order to generate mask: use imfill
% Refer to this webpage to learn about xml in matlab
% http://blogs.mathworks.com/desktop/2010/11/01/xml-and-matlab-navigating-a
% -tree/
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%TURN IT OFF, if you don't want to display
DISPLAY = 1;

%directory containing xml file.
breakInd    = regexp(xmlFileName, '/');

if isempty(breakInd)
    xmlFileDir = './';
else
    xmlFileDir  = xmlFileName(1:breakInd(end));
end

docNode = xmlread(xmlFileName);
CCNode  = docNode.getDocumentElement;

imgName = char(CCNode.getElementsByTagName('imgName').item(0).getTextContent);

%parse the image name.
[pathstr, name, ext] = fileparts(imgName);
imgName = [name, ext]

try
    img  = imread([xmlFileDir,strtrim(imgName)]);
catch ME
    disp([imgName,' NOT found in the directory containing the xml file']);
    img = [];
end

width  = str2double(char(CCNode.getElementsByTagName('img_w').item(0).getTextContent));
height = str2double(char(CCNode.getElementsByTagName('img_h').item(0).getTextContent));

cc = CCNode.getElementsByTagName('closedContour');
numCC = cc.getLength;
closedContours = cell(numCC,1);
for i=0:numCC-1 
    ptXNodes = cc.item(i).getElementsByTagName('x');    
    ptYNodes = cc.item(i).getElementsByTagName('y');    
    contourLength = ptXNodes.getLength;   
    xy_temp = [];
    % first element and last element correspond to fixation point and bbbox. 
    for j=1:contourLength-2
        xy_temp = [xy_temp; str2num(char(ptXNodes.item(j).getTextContent)) str2num(char(ptYNodes.item(j).getTextContent))];       
    end
    closedContours{i+1} = xy_temp;
end


% create masks
masks = cell(length(closedContours),1);
for i=1:length(closedContours)
    bwImg = zeros(height,width);
    ind = sub2ind(size(bwImg), closedContours{i}(:,2) , closedContours{i}(:,1));
    bwImg(ind)=255;
    bwImg = imfill(bwImg);
    masks{i} = bwImg;
end

%display
if DISPLAY
    drawSegments(img, masks);
end
end
