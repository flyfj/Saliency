
%% prepare

% addpath(genpath('C:\vlfeat\'));


%% load training data

datapath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';
allfn = dir([datapath '*.jpg']);
imgnum = length(allfn);

traindatafile = 'nyuboundary.mat';

possamps = [];
negsamps = [];

newsz = [300, 300];

for i=1:imgnum
    [~, fn, ~] = fileparts(allfn(i).name);
    cimgfn = [datapath fn '.jpg'];
    dmapfn = [datapath fn '_d.mat'];
    limgfn = [datapath fn '_l.png'];
    cimg = imread(cimgfn);
    %cimg = imresize(cimg, newsz);
    dmap = load(dmapfn);
    dmap = dmap.depth;
    %dmap = imresize(dmap, newsz);
    limg = imread(limgfn);
    %limg = imresize(limg, newsz);
    
    % extract sample points
    ratio = 0.1;
    maxlabel = max(max(limg));
    boundaryPts = cell(maxlabel, 1);
    nonboundaryPts = cell(maxlabel, 1);
    for j=1:maxlabel
        selobj = limg==j;
        selobj = edge(selobj, 'canny');
        % extract object boundary points as positive samples
        [posy, posx] = find(selobj == 1);
        sel_num = max(1, int32(length(posy)*ratio));
        pos_ids = randperm(length(posy), sel_num);
        pospts = [posx posy];
        boundaryPts{j, 1} = pospts(pos_ids, :);
        % select same number of negative points
        [negy, negx] = find(selobj == 0);
        negpts = [negx negy];
        neg_ids = randperm(length(negy), sel_num);
        nonboundaryPts{j, 1} = negpts(neg_ids, :);
        
        clearvars posy posx pospts negy negx negpts
    end

    % compute feature maps
    cgrad = compGrad2(cimg);
    dgrad = compGrad2(dmap);
    ngrad = compNormalMap(dmap);
    
    cgrad = uint8( getnormimg(cgrad) .* 255 );
    dgrad = uint8( getnormimg(dgrad) .* 255 );
    ngrad = uint8( getnormimg(ngrad) .* 255 );
    
    % extract point descriptors for positive and negative points
    % FREAK
    for j=1:size(boundaryPts, 1)
        cpts = SURFPoints(boundaryPts{j,1});
        [cfeats, ~] = extractFeatures(cgrad, cpts, 'Method', 'SURF');
        [dfeats, ~] = extractFeatures(dgrad, cpts, 'Method', 'SURF');
        [nfeats, ~] = extractFeatures(ngrad, cpts, 'Method', 'SURF');
        possamps = [possamps; cfeats.Features dfeats.Features nfeats.Features];
    end
    for j=1:size(nonboundaryPts, 1)
        cpts = cornerPoints(nonboundaryPts{j,1});
        [cfeats, ~] = extractFeatures(cgrad, cpts, 'Method', 'FREAK');
        [dfeats, ~] = extractFeatures(dgrad, cpts, 'Method', 'FREAK');
        [nfeats, ~] = extractFeatures(ngrad, cpts, 'Method', 'FREAK');
        negsamps = [negsamps; cfeats.Features dfeats.Features nfeats.Features];
    end
    
end

% save data
save(traindatafile, 'possamps', 'negsamps');

%% train boundary classifier

if ~exists('traindata', 'var')
    load(traindatafile);
end


