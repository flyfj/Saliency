
%PARSENYUDATATOFILES Summary of this function goes here
%   Detailed explanation goes here

savepath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';

% parse nyu data
nyudata = load('E:\Datasets\RGBD_Dataset\NYU\nyu_depth_v2_labeled.mat');
nyuimgs = nyudata.images;
nyudepth = nyudata.depths;
nyulabels = nyudata.labels;
clearvars nyudata

% get boundary points for each image
[imgh, imgw, imgnum] = size(nyudepth);

% output images
for i=1:imgnum
    imgfn = [savepath num2str(i) '.jpg'];
    imwrite(nyuimgs(:,:,:,i), imgfn, 'JPEG');
    depthfn = [savepath num2str(i) '_d.txt'];
    imgfn = [savepath num2str(i) '_d.mat'];
    depth = nyudepth(:,:,i);
    fid = fopen(depthfn, 'w');
    fprintf(fid, '%d %d\n', imgw, imgh);
    fprintf(fid, '%d ', depth');
    fclose(fid);
    %save(imgfn, 'depth');
    imgfn = [savepath num2str(i) '_l.png'];
    imwrite(nyulabels(:,:,i), imgfn, 'PNG', 'BitDepth', 16);
    
    disp(['Finished ' num2str(i)]);
end

