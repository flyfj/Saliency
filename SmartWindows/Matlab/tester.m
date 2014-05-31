


newsz = [300, 300];

datapath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';
fnlist = dir([datapath '*.jpg']);
rnd_fnids = randperm(size(fnlist, 1), 10);

topboxnum = [100 300 500 800 1000 1500 2000];

tot_gtnum = 0;
det_gtnum = zeros(1, length(topboxnum));

for i=1:length(rnd_fnids)

%% compute object proposal for a given image
fn = num2str(rnd_fnids(i));

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
tot_gtnum = tot_gtnum + size(gtboxes, 1);

% get object proposals
objs = proposeObjsForImg(cimg, dmap);

%% compute precision and recall

for id=1:length(topboxnum)

    vnum = min(size(objs,1), topboxnum(id));
    selobjs = objs(1:vnum, :);
    gtcount = zeros(1, size(gtboxes,1));   % number of matched gt objects

    for j=1:size(selobjs,1)
        curobj = [selobjs(j,1) selobjs(j,2) selobjs(j,3)-selobjs(j,1) selobjs(j,4)-selobjs(j,2)];
        for k=1:size(gtboxes,1)
            curgt = [gtboxes(k,1) gtboxes(k,2) gtboxes(k,3)-gtboxes(k,1) gtboxes(k,4)-gtboxes(k,2)];
            area = rectint(curgt, curobj);
            area = double(area);
            if area > 0
                % compute union area
                uarea = ( max(objs(j,3), gtboxes(k,3))-min(curobj(1), curgt(1)) ) * ( max(objs(j,4), gtboxes(k,4))-min(curobj(2), curgt(2)) );
                uarea = double(uarea);
                if area / uarea >= 0.5
                    gtcount(k) = gtcount(k) + 1;
                end
            end
        end
    end

    det_gtnum(id) = det_gtnum(id) + sum(gtcount>0);

end


end

%% draw recall curve

recall_vals = zeros(1, length(topboxnum));
for i=1:length(topboxnum)
    recall_vals(i) = det_gtnum(i) / tot_gtnum;
end

figure
plot(1:i, recall_vals, 'r-')
pause
close all








