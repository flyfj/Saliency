
% params
posdir = 'E:\Datasets\objectness\pos\';
negdir = 'E:\Datasets\objectness\neg\';


%% load data and compute features

if exist('objdata.mat', 'file')

    objdata = load('objdata.mat');
    posdata = objdata.posdata;
    negdata = objdata.negdata;

else

% positive samples
poscimgs = dir([posdir '*.jpg']);
poscn = length(poscimgs);

showimg = 0;

posdata = zeros(poscn, 64);
for i=1:poscn
    cfn = poscimgs(i).name;
    cimg = imread([posdir cfn]);
    [~, tfn, ~] = fileparts(cfn);
    dfn = [tfn '_d.txt'];
    dimg = load([posdir dfn]);
    dimg = double(dimg);
    
    assert(size(cimg,1) == size(dimg, 1) && size(cimg, 2) == size(dimg, 2));
    
    gradimg = compRGBDGrad(cimg, dimg);
    gradimg = imresize(gradimg, [8, 8]);
    posdata(i, :) = gradimg(:);
    dimg = dimg ./ max(dimg(:));
    
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

negdata = zeros(negcn, 64);
for i=1:negcn
    cfn = negcimgs(i).name;
    cimg = imread([negdir cfn]);
    [~, tfn, ~] = fileparts(cfn);
    dfn = [tfn '_d.txt'];
    dimg = load([negdir dfn]);
    dimg = double(dimg);
    
    gradimg = compRGBDGrad(cimg, dimg);
    gradimg = imresize(gradimg, [8, 8]);
    negdata(i, :) = gradimg(:);
    dimg = dimg ./ max(dimg(:));
    
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

save('objdata.mat', 'posdata', 'negdata');

end

%% train svm

addpath('libsvm');
% select 80% samples as training data
posnum = size(posdata, 1);
posbound = floor(posnum*0.8);
negnum = size(negdata, 1);
negbound = floor(negnum*0.8);
traindata = [posdata(1:posbound, :); negdata(1:negbound, :)];
testdata = [posdata(posbound+1:end, :); negdata(negbound+1, :)];
trainlabels = [ones(posbound, 1); zeros(negbound, 1)-1];
testlabels = [ones(posnum-posbound+1, 1); zeros(negnum-negbound+1, 1)-1];

svmmodel = svmtrain( trainlabels, traindata, '-t 0');


%% testing

[pred_labels, accuracy, scores] = svmpredict(testlabels, testdata, svmmodel);
