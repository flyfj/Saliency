
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