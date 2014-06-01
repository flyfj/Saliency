function objs = cvpr_predict( cimg )
%CVPR_PREDICT Summary of this function goes here
%   use cvpr14 gradient based model to predict objectness
%   use oversegmentation to produce candiates

% model file
modelfn = 'cvpr14_b3d_model.mat';
model = load(modelfn);
model = model.model;

addpath(genpath('Graph_seg'));
ths = [0.1 0.5 1 2 3 5];

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
        nbox = [minx, miny, maxx-minx, maxy-miny];
        subimg = imcrop(cimg, nbox);
        subimg = imresize(subimg, [8 8]);
        cgradimg = compGrad(subimg);
        candidates = [candidates; cgradimg(:)' box];
    end
    
    disp(['Th: ' num2str(ths(i)) ' - processed: ' num2str(length(segids)) ' objects.']);
end

% classify
tlabels = zeros(size(candidates, 1), 1);
[~, ~, scores] = predict(tlabels, sparse(candidates(:, 1:64)), model);

objs = [candidates(:, 65:end) scores];

% sort segments by values
objs = sortrows(objs, -5);


end

