
clear

savepath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';

% parse nyu data
nyudepth = load('E:\Datasets\RGBD_Dataset\NYU\nyu_depth_v2_labeled.mat');

allcimgs = nyudepth.images;
alldimgs = nyudepth.depths;
alllabels = nyudepth.labels;

[imgh, imgw, imgnum] = size(allcimgs);

% convert to point cloud
compNormalMap(alldimgs(:,:,1));



