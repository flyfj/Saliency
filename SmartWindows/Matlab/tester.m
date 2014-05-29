
newsz = [300, 300];

datapath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';
fn = '841';

cimgfn = [datapath fn '.jpg'];
cimg = imread(cimgfn);
% cimg = imresize(cimg, newsz);

dmapfn = [datapath fn '_d.mat'];
dmap = load(dmapfn);
dmap = dmap.depth;
% dmap = imresize(dmap, newsz);

limgfn = [datapath fn '_l.png'];
limg = imread(limgfn);
% limg = imresize(limg, newsz);

bmap = compBoundaryMap(cimg, dmap);

% show
imshow(bmap, [])
colormap jet
colorbar
pause

% propose object segments / windows
addpath('Graph_seg');
ths = [0.1 0.5 1 1.5 2 3 5];
segimgs = cell(length(ths), 1);

for i=1:length(ths)
    [L, ~] = graph_segment(cimg, 1, ths(i), 100);
    segids = unique(L(:));
    for j=1:length(segids)
        
    end
end



