function [ output_args ] = draw_rgbd( cimgf, dmapf )
%DRAW_RGBD Summary of this function goes here
%   Detailed explanation goes here

datadir = 'E:\Datasets\RGBD_Dataset\Berkeley\VOCB3DO\KinectColor\';
cimgfn = [datadir cimgf];
dmapfn = [datadir dmapf];
cimg = imread(cimgfn);
dmap = imread(dmapfn);

%% to visualize rgbd images

scale = 0.5;

cimg = imresize(cimg, scale);

% visualize in a row
fig = figure(1)
subplot(1,2,1)
imshow(cimg)

subplot(1,2,2)
dmap = imresize(dmap, scale);
dmap = getnormimg(dmap);
imshow(dmap)
colormap jet

pause

close all


end

