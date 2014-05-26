function [nyuimgs, nyudepth, nyuboundary] = parsenyudata()

savepath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';

% parse nyu data
if ~exist('nyudata', 'var')
    nyudata = load('E:\Datasets\RGBD_Dataset\NYU\nyu_depth_v2_labeled.mat');
end
nyuimgs = nyudata.images;
nyudepth = nyudata.depths;
nyulabels = nyudata.labels;

clearvars nyudata

% get boundary points for each image
[imgh, imgw, imgnum] = size(nyuimgs);

nyuboundary = cell(imgnum, 1);
for i=1:imgnum
    curlabel = nyulabels(:,:,i);
    maxid = max(max(curlabel));
    curcell = cell(maxid, 1);
    for j=1:maxid
        selobj = curlabel==j;
        selobj = edge(selobj, 'canny');
        [y, x] = find(selobj == 1);
        curcell{j, 1} = [x y];
    end
    nyuboundary{i, 1} = curcell;
    
    disp(['Finished ' num2str(i)]);
end





