
% params
posdir = 'E:\Datasets\objectness\b3d_pos\';
negdir = 'E:\Datasets\objectness\b3d_neg\';


%% load data and compute features
datafile = 'objdata_b3d_color.mat';
usedepth = 1;
usecolor = 1;
usenormal = 1;

showimg = 0;

if exist(datafile, 'file')

    objdata = load(datafile);
    posdata = objdata.posdata;
    negdata = objdata.negdata;

else

% positive samples
poscimgs = dir([posdir '*.jpg']);
poscn = length(poscimgs);
poscn = floor(poscn / 1);

posdata = zeros(poscn, 64);
for i=1:poscn
    cfn = poscimgs(i).name;
    cimg = imread([posdir cfn]);
    [~, tfn, ~] = fileparts(cfn);
    dimg = [];
    
    %assert(size(cimg,1) == size(dimg, 1) && size(cimg, 2) == size(dimg, 2));
    
    if usecolor == 1
        cimgs = imresize(cimg, [8 8]);
        cgradimg = compGrad(cimgs);
%         posdata(i, :) = cgradimg(:);
    end
    if usedepth == 1
        dfn = [tfn '_d.txt'];
        dimg = load([posdir dfn]);
        dimg = double(dimg);
        dimg = imresize(dimg, [8 8]);
        dgradimg = compGrad(dimg);
%         posdata(i, :) = dgradimg(:);
    end
    if usenormal == 1
        
        gradimg = getnormimg(cgradimg) + getnormimg(dgradimg);
        posdata(i,:) = gradimg(:);
        
%         dfn = [tfn '_d.txt'];
%         dimg = load([posdir dfn]);
%         dimg = double(dimg);
%         nmap = computeNormalMap(dimg);
%         nmap = imresize(nmap, [8 8]);
%         gradimg = compGrad(nmap);
%         posdata(i, :) = gradimg(:);
    end
   
    gradimg = imresize(gradimg, [64, 64]);
    %dimg = dimg ./ max(dimg(:));
    
    if showimg == 1
        figure(1)
        imshow(cimg);
        if usedepth == 1
            figure(2)
            dimg = getnormimg(dimg);
            imshow(dimg);
        end
        figure(3)
        gradimg = getnormimg(gradimg);
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
    
    if usecolor == 1
        cimgs = imresize(cimg, [8 8]);
        cgradimg = compGrad(cimgs);
%         posdata(i, :) = cgradimg(:);
    end
    if usedepth == 1
        dfn = [tfn '_d.txt'];
        dimg = load([negdir dfn]);
        dimg = double(dimg);
        dimg = imresize(dimg, [8 8]);
        dgradimg = compGrad(dimg);
%         negdata(i, :) = dgradimg(:);
    end
     if usenormal == 1
        gradimg = getnormimg(cgradimg) + getnormimg(dgradimg);
        negdata(i,:) = gradimg(:);
        
%         dfn = [tfn '_d.txt'];
%         dimg = load([negdir dfn]);
%         dimg = double(dimg);
%         nmap = computeNormalMap(dimg);
%         nmap = imresize(nmap, [8 8]);
%         gradimg = compGrad(nmap);
%         posdata(i, :) = gradimg(:);
    end
    
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

save('cvpr14_b3d_model.mat', 'model');

%% testing

%C = svmclassify(svmStruct, testdata);
% SampleScaleShift = bsxfun(@plus, testdata, svmStruct.ScaleData.shift);
% Sample = bsxfun(@times, SampleScaleShift, svmStruct.ScaleData.scaleFactor);
% sv = svmStruct.SupportVectors;
% alphaHat = svmStruct.Alpha;
% bias = svmStruct.Bias;
% kfun = svmStruct.KernelFunction;
% kfunargs = svmStruct.KernelFunctionArgs;
% scores = kfun(sv, Sample, kfunargs{:})'*alphaHat(:) + bias;
% scores = scores*-1;
[predicted_label, accuracy, scores] = predict(testlabels, sparse(testdata), model);
% save('color_scores.mat', 'scores');
pos_accu = sum(predicted_label(1:(posnum-posbound), 1) == 1) / (posnum-posbound)
neg_accu = sum(predicted_label((posnum-posbound+1):end, 1) == -1) / (negnum-negbound)

% matlab svm
% y = trainlabels(svmStruct.SupportVectorIndices);
% w = (svmStruct.Alpha' ) * svmStruct.SupportVectors;
% w = reshape(w, [8 8]);
% w(w<0) = 0;
% w = getnormimg(w);
% w = imresize(w, [64 64]);
% imshow(w)

% show w
w = model.w;
w = reshape(w, [8 8]);
w(w<0) = 0;
w = imresize(w, [64 64]);
w = getnormimg(w);
figure
imshow(w)
%save('normal_w', 'w');


% err_rate = sum(testlabels ~= C) / length(testlabels);


