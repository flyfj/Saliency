
%% prepare

% addpath(genpath('C:\vlfeat\'));

datapath = 'E:\Datasets\RGBD_Dataset\NYU\Depth2\';
traindatafile = 'nyuboundary.mat';

totrain = 1;

%% load training data

if ~exist(traindatafile, 'file') || totrain==1

useimg = 20;
sampperimg = 100;
    
allfn = dir([datapath '*.jpg']);
imgnum = length(allfn);
sel_imgs = randperm(imgnum, useimg);

possamps = [];
negsamps = [];

newsz = [300, 300];

for i=1:length(sel_imgs)
    curimgid = sel_imgs(i);
    [~, fn, ~] = fileparts(allfn(curimgid).name);
    cimgfn = [datapath fn '.jpg'];
    dmapfn = [datapath fn '_d.mat'];
    limgfn = [datapath fn '_l.png'];
    cimg = imread(cimgfn);
%     cimg = imresize(cimg, newsz);
    dmap = load(dmapfn);
    dmap = dmap.depth;
%     dmap = imresize(dmap, newsz);
    limg = imread(limgfn);
%     limg = imresize(limg, newsz);
    
    % extract sample points
    ratio = 0.1;
    validlabels = unique(limg(:));
    % remove background 0
    if validlabels(1) == 0
        validlabels(1) = [];
    end
    boundaryPts = cell(length(validlabels), 1);
    nonboundaryPts = cell(length(validlabels), 1);
    
    for j=1:length(validlabels)
        curid = validlabels(j);
        selobj = limg==curid;
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
        sel_num = max(1, int32(length(negy)*ratio));
        neg_ids = randperm(length(negy), sel_num);
        nonboundaryPts{j, 1} = negpts(neg_ids, :);
        
        clearvars posy posx pospts negy negx negpts
    end

    % compute feature maps
%     dmap = medfilt2(dmap, [3 3]);
%     dmap = filter2(fspecial('average',3), dmap);
%     dmap = filter2(fspecial('average',3), dmap);
    [cgrad, dgrad, ngrad] = compFeatMaps(cimg, dmap, 1);
    
    % extract point descriptors for positive and negative points
    % FREAK
    curpossamps = [];
    for j=1:size(boundaryPts, 1)
        cpts = SURFPoints(boundaryPts{j,1});
        [cfeats, ~] = extractFeatures(cgrad, cpts, 'Method', 'SURF');
        [dfeats, ~] = extractFeatures(dgrad, cpts, 'Method', 'SURF');
        [nfeats, ~] = extractFeatures(ngrad, cpts, 'Method', 'SURF');
        curpossamps = [curpossamps; cfeats dfeats nfeats];
    end
    validnum = min(size(curpossamps, 1), sampperimg);
    curpossamps = curpossamps(randperm(size(curpossamps, 1), validnum), :);
    possamps = [possamps; curpossamps];
    
    curnegsamps = [];
    for j=1:size(nonboundaryPts, 1)
        cpts = SURFPoints(nonboundaryPts{j,1});
        [cfeats, ~] = extractFeatures(cgrad, cpts, 'Method', 'SURF');
        [dfeats, ~] = extractFeatures(dgrad, cpts, 'Method', 'SURF');
        [nfeats, ~] = extractFeatures(ngrad, cpts, 'Method', 'SURF');
        curnegsamps = [curnegsamps; cfeats dfeats nfeats];
    end
    validnum = min(size(curnegsamps, 1), sampperimg);
    curnegsamps = curnegsamps(randperm(size(curnegsamps, 1), validnum), :);
    negsamps = [negsamps; curnegsamps];
    
    disp(['Sampled from image ' num2str(i)]);
    
end

% save data
save(traindatafile, 'possamps', 'negsamps');

end

%% train boundary classifier

if ~exist('possamps', 'var')
    load(traindatafile);
end

possamps = double(possamps);
negsamps = double(negsamps);

addpath(genpath('liblinear'));
% select subset of samples as training data
ratio = 0.8;
posnum = size(possamps, 1);
posbound = floor(posnum*ratio);
negnum = size(negsamps, 1);
negbound = floor(negnum*ratio);
traindata = [possamps(1:posbound, :); negsamps(1:negbound, :)];
testdata = [possamps(posbound+1:end, :); negsamps(negbound+1:end, :)];
trainlabels = [ones(posbound, 1); zeros(negbound, 1)-1];
testlabels = [ones(posnum-posbound, 1); zeros(negnum-negbound, 1)-1];

% train
options.MaxIter = 150000;
model = train(trainlabels, sparse(traindata));
% svmStruct = svmtrain(traindata, trainlabels, 'kernel_function', 'mlp');
save('boundaryClf.mat', 'model');

% test
[predicted_label, accuracy, scores] = predict(testlabels, sparse(testdata), model);

pos_accu = sum(predicted_label(1:(posnum-posbound), 1) == 1) / (posnum-posbound)
neg_accu = sum(predicted_label((posnum-posbound+1):end, 1) == -1) / (negnum-negbound)
