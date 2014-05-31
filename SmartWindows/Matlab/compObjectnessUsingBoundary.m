function val = compObjectnessUsingBoundary( segmask, bmap )
%COMPOBJECTNESSUSINGBOUNDARY Summary of this function goes here
%   Detailed explanation goes here

% measure 1: ratio of boundary points w.r.t. segment boundary

% get segment contour pixels
se_thin = strel('square', 2);
se_thick = strel('square', 4);
in_mask_big = imerode(segmask, se_thin);
in_mask_small = imerode(segmask, se_thick);
contour_mask = segmask ~= in_mask_big;
contour_range_mask = segmask ~= in_mask_small;

posbmap = bmap>=0;
bmask = contour_range_mask & posbmap;
contourbsum = sum(bmap(bmask>0));
seg_perimeter = sum(sum(contour_mask>0));

val = contourbsum ./ seg_perimeter;

end

