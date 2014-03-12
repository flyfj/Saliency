
%% to visualize rgbd images as 3d

clear

usedepth = 1;

imgfile = 'E:\Datasets\RGBD Dataset\NYU\img\115_img.png';
depthfile = 'E:\Datasets\RGBD Dataset\NYU\img\115_depth.txt';

scale = 0.4;

img = imread(imgfile);
img = imresize(img, scale);
% mat3d(img)
% pause
[m ,n,c] = size(img);
X = repmat(1:n, m, 1);
Y = repmat((1:m)', 1, n);

% convert to gray
grayimg = rgb2gray(img);
% grayimg = double(grayimg);

dmap = load(depthfile);
dmap = 1./ dmap;
dmap = imresize(dmap, scale);

% visualize in a row
figure(1)
subplot(2,1,1)
imshow(img)
subplot(2,1,2)
imshow(grayimg)

figure(2)
surf(grayimg, 'FaceColor', 'texturemap')
colormap jet
colorbar

if usedepth
    figure(3)
    surf(dmap, 'FaceColor', 'texturemap')
    colormap jet
    colorbar
end

pause
close all