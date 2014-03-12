
%% to visualize rgbd images as 3d

clear

usedepth = 1;

imgfile = 'E:\Datasets\RGBD Dataset\NYU\img\115_img.png';
depthfile = 'E:\Datasets\RGBD Dataset\NYU\img\115_depth.txt';

img = imread(imgfile);
mat3d(img)
pause
[m ,n,c] = size(img);
% m = 100, n = 100;
X = repmat(1:n, m, 1);
Y = repmat((1:m)', 1, n);

% convert to gray
grayimg = rgb2gray(img);
grayimg = double(grayimg);
grayimg = imresize(grayimg, 0.2);

dmap = load(depthfile);

% visualize in a row
figure(1)
% [xx, yy] = meshgrid(1: size(grayimg, 1), 1: size(grayimg, 2));
% scatter3(xx(:), yy(:), grayimg(:), grayimg(:));
surf(grayimg, 'EdgeColor','none')
colormap jet
hold on

subplot(1,3,1)
imshow(img)
hold on

subplot(1,3,2)
surf(X, Y, grayimg)
hold on

if usedepth
    subplot(1,3,3)
    surf(X, Y, dmap)
    hold on
end

pause
close all