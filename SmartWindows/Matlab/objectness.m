
% params
posdir = 'E:\Datasets\objectness\voc_pos\';
negdir = 'E:\Datasets\objectness\voc_neg\';


%% load data and compute features
datafile = 'vod_objdata.mat';
usedepth = 0;

if exist(datafile, 'file')

    objdata = load(datafile);
    posdata = objdata.posdata;
    negdata = objdata.negdata;

else

% positive samples
poscimgs = dir([posdir '*.jpg']);
poscn = length(poscimgs);
poscn = floor(poscn / 100);

showimg = 0;

posdata = zeros(poscn, 64);
for i=1:poscn
    cfn = poscimgs(i).name;
    cimg = imread([posdir cfn]);
    [~, tfn, ~] = fileparts(cfn);
    dimg = [];
    if(usedepth==1)
        dfn = [tfn '_d.txt'];
        dimg = load([posdir dfn]);
        dimg = double(dimg);
    end
    
    %assert(size(cimg,1) == size(dimg, 1) && size(cimg, 2) == size(dimg, 2));
    
    gradimg = compRGBDGrad(cimg, dimg);
    gradimg = imresize(gradimg, [8, 8]);
    posdata(i, :) = gradimg(:);
    gradimg = gradimg ./ max(gradimg(:));
    gradimg = imresize(gradimg, [64, 64]);
    %dimg = dimg ./ max(dimg(:));
    
    if showimg == 1
        figure(1)
        imshow(cimg);
        figure(2)
        imshow(dimg);
        figure(3)
        imshow(gradimg);
        pause
        close all
    end
    
    fprintf('pos: %d / %d\n', i, poscn);

end


% negative samples
negcimgs = dir([negdir '*.jpg']);
negdimgs = dir([negdir '*.txt']);
negcn = length(negcimgs);
negdn = length(negdimgs);
negcn = floor(negcn / 100);

negdata = zeros(negcn, 64);
for i=1:negcn
    cfn = negcimgs(i).name;
    cimg = imread([negdir cfn]);
    [~, tfn, ~] = fileparts(cfn);
    dimg = [];
    if(usedepth == 1)
        dfn = [tfn '_d.txt'];
        dimg = load([negdir dfn]);
        dimg = double(dimg);
    end
    
    gradimg = compRGBDGrad(cimg, dimg);
    gradimg = imresize(gradimg, [8, 8]);
    negdata(i, :) = gradimg(:);
%     dimg = dimg ./ max(dimg(:));
    
    if showimg == 1
        figure(1)
        imshow(cimg);
        figure(2)
        imshow(dimg);
        figure(3)
        imshow(gradimg);
        pause
        close all
    end
    
    fprintf('neg: %d / %d\n', i, negcn);
    
end

save(datafile, 'posdata', 'negdata');

end

%% train svm

%addpath(genpath('libsvm'));
% select 80% samples as training data
posnum = size(posdata, 1);
posbound = floor(posnum*0.8);
negnum = size(negdata, 1);
negbound = floor(negnum*0.8);
traindata = [posdata(1:posbound, :); negdata(1:negbound, :)];
testdata = [posdata(posbound+1:end, :); negdata(negbound+1:end, :)];
trainlabels = [ones(posbound, 1); zeros(negbound, 1)-1];
testlabels = [ones(posnum-posbound, 1); zeros(negnum-negbound, 1)-1];

options.MaxIter = 150000;
svmStruct = svmtrain(traindata, trainlabels);


%% testing

C = svmclassify(svmStruct, testdata);
err_rate = sum(testlabels ~= C) / length(testlabels);


