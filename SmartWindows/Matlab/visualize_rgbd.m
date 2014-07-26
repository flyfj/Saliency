function [] = visualize_rgbd(cimgf, dmapf)
% input: color image and depth map

datadir = 'E:\Datasets\RGBD_Dataset\Berkeley\VOCB3DO\KinectColor\';
cimgfn = [datadir cimgf];
dmapfn = [datadir dmapf];
cimg = imread(cimgfn);
dmap = imread(dmapfn);

%% to visualize rgbd images as 3d

if ~exist('dmap', 'var')
    usedepth = 0;
else
    usedepth = 1;
end

scale = 0.3;

cimg = imresize(cimg, scale);

if length(size(cimg)) == 2
    grayimg = cimg;
else
    % convert to gray
    grayimg = rgb2gray(cimg);
    % grayimg = double(grayimg);
end

% visualize in a row
figure(1)
subplot(2,1,1)
imshow(cimg)
subplot(2,1,2)
imshow(grayimg)

figure(2)
surf(grayimg, 'FaceColor', 'texturemap')
colormap jet
colorbar

if usedepth
  
    dmap = 1./ dmap;
    dmap = imresize(dmap, scale);
    
    figure(3)
    surf(dmap, 'FaceColor', 'texturemap')
    colormap jet
    colorbar
end

pause
close all

end