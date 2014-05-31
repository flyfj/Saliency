function boxes = getGTBoxFromLabels( limg )
%GETGTBOXFROMLABELS Summary of this function goes here
%   Detailed explanation goes here

labels = unique(limg(:));
% remove background 0
if labels(1)==0
    labels(1) = [];
end

boxes = zeros(length(labels), 4);

for i=1:length(labels)
    segmask = limg==labels(i);
    [posy, posx] = find(segmask==1);
    minx = min(posx);
    maxx = max(posx);
    miny = min(posy);
    maxy = max(posy);
    boxes(i,:) = [minx miny maxx maxy];
end

