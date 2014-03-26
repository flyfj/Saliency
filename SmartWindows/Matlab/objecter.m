

%% detect object

img = imread('tiger.jpg');
[m,n] = size(img);

winsz = [50 50];

grayimg = rgb2gray(img);

[gx, gy] = imgradientxy(grayimg, 'prewitt');

figure(1)
imshow(grayimg)
hold on
plot()