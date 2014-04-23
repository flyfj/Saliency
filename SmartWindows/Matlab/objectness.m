
% params
posdir = 'E:\Datasets\objectness\pos\';
negdir = 'E:\Datasets\objectness\neg\';


%% load data and compute features

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
end

% negative samples
negcimgs = dir([negdir '*.jpg']);
negdimgs = dir([negdir '*.txt']);
negcn = length(negcimgs);
negdn = length(negdimgs);

negdata = zeros(negcn, 64);
for i=1:negcn
    cfn = negcimgs(i).name;
    cimg = imread([posdir cfn]);
    [~, tfn, ~] = fileparts(cfn);
    dfn = [tfn '_d.txt'];
    dimg = load([negdir dfn]);
    dimg = double(dimg);
    
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
end

save('objdata.mat', 'posdata', 'negdata');


%% train svm



%% testing

