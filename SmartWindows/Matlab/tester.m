

datapath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';
cimgfn = [datapath '4.jpg'];
cimg = imread(cimgfn);

dmapfn = [datapath '4_d.mat'];
dmap = load(dmapfn);
dmap = dmap.depth;

limgfn = [datapath '4_l.png'];
limg = imread(limgfn);

bmap = compBoundaryMap(cimg, dmap);


% show
imshow(bmap, [])
pause