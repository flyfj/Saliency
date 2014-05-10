
% params
posdir = 'E:\Datasets\objectness\voc07_pos\';
negdir = 'E:\Datasets\objectness\voc07_neg\';


%% load data and compute features
datafile = 'objdata_voc07color.mat';
usedepth = 0;

if exist(datafile, 'file')

    objdata = load(datafile);
    posdata = objdata.posdata;
    negdata = objdata.negdata;

else

% positive samples
poscimgs = dir([posdir '*.jpg']);
poscn = length(poscimgs);
poscn = floor(poscn / 1);

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
    
    cimg = imresize(cimg, [8 8]);
    gradimg = compGrad(cimg);
    %gradimg = imresize(gradimg, [8, 8]);
    posdata(i, :) = gradimg(:);
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
negcn = floor(negcn / 1);

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

addpath(genpath('liblinear'));

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
model = train(trainlabels, sparse(traindata));
% svmStruct = svmtrain(traindata, trainlabels, 'kernel_function', 'mlp');


%% testing

[predicted_label, accuracy, scores] = predict(testlabels, sparse(testdata), model);
pos_accu = sum(predicted_label(1:(posnum-posbound), 1) == 1) / (posnum-posbound)
neg_accu = sum(predicted_label((posnum-posbound):end, 1) == -1) / (negnum-negbound)

% C = svmclassify(svmStruct, testdata);
% err_rate = sum(testlabels ~= C) / length(testlabels);


