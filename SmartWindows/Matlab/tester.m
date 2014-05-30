
newsz = [300, 300];

datapath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';
fn = '843';

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

%% propose object segments / windows
addpath(genpath('Graph_seg'));
ths = [0.1 0.5 1 2 3 5];
segimgs = cell(length(ths), 1);

candidates = [];

for i=1:length(ths)
    [L, ~] = graph_segment(cimg, 1, ths(i), 100);
    segids = unique(L(:));
    for j=1:length(segids)
        cursegmask = L==segids(j);
        % bounding box
        [posy, posx] = find(cursegmask==1);
        minx = min(posx);
        maxx = max(posx);
        miny = min(posy);
        maxy = max(posy);
        box = [minx miny maxx maxy];
        % compute objectness value
        val = compObjectnessUsingBoundary(cursegmask, bmap);
        candidates = [candidates; box val];
    end
    
    disp(['Th: ' num2str(ths(i)) ' - processed: ' num2str(length(segids)) ' objects.']);
end

% sort segments by values
objs = sortrows(candidates, -5);

%%  show top candidates
dispimg = cimg;
imshow(dispimg)
hold on
for i=1:500 
    plot([objs(i,1) objs(i,3) objs(i,3) objs(i,1) objs(i,1)], [objs(i,2) objs(i,2) objs(i,4) objs(i,4) objs(i,2)], 'r-')
    hold on
end

pause
close all


