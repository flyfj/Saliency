function [] = mat3d( im )
%MAT3D Summary of this function goes here
%   Detailed explanation goes here

sz=size(im);
ix=sz(1);
iy=sz(2);

% filter the image
h=fspecial('average',5);
filtim=imfilter(im,h);

% create reference surface
img2 = imresize(im, 0.2);
[x, y] = meshgrid(size(img2));
% [x,y,z]=peaks(30);
if size(im, 3) == 1; z = im; else z = rgb2gray(im); end
sz=size(z);
sx=sz(1);
sy=sz(2);

% scaling
xfkt=sx/ix;
yfkt=sy/iy;

% plot image on surface
warp(z,filtim)
hold on;

% compute isoline for red level
% and plot 1st isoline only
rlevel=0.84
[c,h]=contour(filtim(:,:,1),[rlevel]);
idx=2:c(2,1);
plot3(c(1,idx)*xfkt,c(2,idx)*yfkt,idx*0+5,'b.')
axis([0 sx 0 sy -10 10])
title(['cut-level: ' num2str(rlevel)])

end

