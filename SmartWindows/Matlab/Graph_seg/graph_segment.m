function [L, contour] = graph_segment(img, neighbor_radius, coefficient, min_size)
%Inputs:
%img:  input image for segmentation
%neighbor_radius: the neighborhood radius of each pixel[1 by default]
%coefficient: segmentation algorithm coefficient(large prefert large segmented component)
%min_size: the minimum size allowed for each segment.

%Outputs:
%L: labeled image
%contour: 

%add the neccessary path:
addpath('CoherenceFilter');
%normalize data:
img = im2double(img);
img = CoherenceFilter(img,struct('T',5,'rho',.5,'Scheme','R'));
%regulartion:
n_channel = size(img, 3);
for i= 1:n_channel
    img_component = img(:, :, i);
    img_min = min(img_component(:));
    img_range = max(img_component(:)) - img_min;
    img_component = (img_component - img_min) / img_range;
end

L = Graph_seg_gray(img, neighbor_radius, coefficient, min_size);
[L_gx, L_gy] = gradient(L);
L_gradient = L_gx.^2 + L_gy.^2;
contour = (L_gradient > 0);
contour = bwmorph(contour, 'skel');