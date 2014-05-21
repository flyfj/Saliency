

% load scores
cscores = import('color_scores.mat');
dscores = import('depth_scores.mat');
nscores = import('normal_scores.mat');
allscores = [cscores dscores nscores];

datafile = 'objdata_b3d_color.mat';
objdata = load(datafile);
posdata = objdata.posdata;
negdata = objdata.negdata;

posnum = size(posdata, 1);
posbound = floor(posnum*0.8);
negnum = size(negdata, 1);
negbound = floor(negnum*0.8);
traindata = [allscores(1:posbound, :); negdata(1:negbound, :)];
testdata = [posdata(posbound+1:end, :); negdata(negbound+1:end, :)];
trainlabels = [ones(posbound, 1); zeros(negbound, 1)-1];
testlabels = [ones(posnum-posbound, 1); zeros(negnum-negbound, 1)-1];