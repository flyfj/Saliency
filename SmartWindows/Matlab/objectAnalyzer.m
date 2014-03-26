
imgfile = 'E:\Datasets\RGBD Dataset\NYU\img\115_img.png';
depthfile = 'E:\Datasets\RGBD Dataset\NYU\img\115_depth.txt';


%% analyze boundary in images

cimg = imread('red.jpg');

visualize_rgbd(cimg);

grayimg = rgb2gray(cimg);
figure(1)
title('input image')
hold on
subplot(1, 2, 1)
imshow(cimg)
subplot(1, 2, 2)
imshow(grayimg)

% compute edge
% rimg = cimg(:,:,1);
% gimg = cimg(:,:,2);
% bimg = cimg(:,:,3);
% [rgmag, rgdir] = imgradient(rimg, 'sobel');
% [ggmag, ggdir] = imgradient(gimg, 'sobel');
% [bgmag, bgdir] = imgradient(bimg, 'sobel');
% gmag = sqrt(rgmag.^2 + ggmag.^2 + bgmag.^2);

[gx, gy] = imgradientxy(grayimg, 'prewitt');
[gmag, gdir] = imgradient(grayimg, 'prewitt');

figure(2)
title('Gradient magnitude')
hold on
imshow(gmag, [])

% visualize_rgbd(gmag)

figure(3)
title('Gradient direction')
hold on
subplot(1, 3, 1)
imshow(gdir, [])
subplot(1, 3, 2)
imshow(gx, [])
subplot(1, 3, 3)
imshow(gy, [])

figure(4)
[X, Y] = meshgrid(1:2:size(grayimg, 2), 1:2:size(grayimg, 1));
quiver(X(:), Y(:), gx(sub2ind(size(gx), Y(:), X(:))), gy(sub2ind(size(gy), Y(:), X(:))))

pause
close all