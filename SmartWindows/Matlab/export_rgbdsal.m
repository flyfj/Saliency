
% export depth map to png images from RGBD salient object dataset

data_dir = 'E:\Datasets\RGBD_Dataset\Saliency\Depth\smoothedDepth\';
dmapfns = dir([data_dir '*.mat']);

for i=1:length(dmapfns)
    curfn = [data_dir dmapfns(i).name];
    [path, name, ext] = fileparts(curfn);
    dmap = load(curfn);
    imwrite(dmap.smoothedDepth, [path '\' name '.png']);
    disp(['finish' num2str(i)]);
end