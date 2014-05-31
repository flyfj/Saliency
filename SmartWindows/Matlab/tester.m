
% compute object proposal for a given image

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
gtboxes = getGTBoxFromLabels(limg);

% get object proposals
objs = proposeObjsForImg(cimg, dmap, 500);

% compute precision and recall
prval = zeros(1,2);
gtcount = zeros(1, size(gtboxes,1));   % number of matched gt objects
for j=1:size(objs,1)
    curobj = [objs(j,1) objs(j,2) objs(j,3)-objs(j,1) objs(j,4)-objs(j,2)];
    for k=1:size(gtboxes,1)
        curgt = [gtboxes(j,1) gtboxes(j,2) gtboxes(j,3)-gtboxes(j,1) gtboxes(j,4)-gtboxes(j,2)];
        area = recint(curgt, curobj);
        area = double(area);
        if area > 0
            % compute union area
            uarea = ( max(objs(j,3), gtboxes(k,3))-min(curobj(1), curgt(1)) ) * ( max(objs(j,4), gtboxes(k,4))-min(curobj(2), curgt(2)) );
            uarea = double(uarea);
            if area / uarea > 0.5
                gtcount(k) = gtcount(k) + 1;
            end
        end
    end
end

prval(1) = sum(gtcount) / size(objs,1);
prval(2) = sum(gtcount>0) / length(gtcount);




