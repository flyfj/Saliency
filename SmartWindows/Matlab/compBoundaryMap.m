function bmap = compBoundaryMap( cimg, dimg )
%COMPBOUNDARYMAP Summary of this function goes here
%   fuse multiple feature to get a final boundary map

addpath(genpath('liblinear'));

% load model
modelfile = 'boundaryClf.mat';
model = load(modelfile);
model = model.model;

% compute feature maps
[cgrad, dgrad, ngrad] = compFeatMaps(cimg, dimg, 1);

% compute a mask
th = 20;
cgrad(cgrad<th) = 0;
dgrad(dgrad<th) = 0;
ngrad(ngrad<th) = 0;
mask = cgrad + dgrad + ngrad;
mask(mask>0) = 1;


% classify each valid point
[y, x] = find(mask~=0);
pts = [x y];
spts = SURFPoints(pts);
[cfeats, ~] = extractFeatures(cgrad, spts, 'Method', 'SURF');
[dfeats, ~] = extractFeatures(dgrad, spts, 'Method', 'SURF');
[nfeats, ~] = extractFeatures(ngrad, spts, 'Method', 'SURF');
samps = [cfeats dfeats nfeats];
samps = double(samps);

tlabels = ones(size(samps, 1), 1);
[~, ~, scores] = predict(tlabels, sparse(samps), model);

bmap = zeros(size(mask));
for i=1:size(pts,1)
    bmap(pts(i,2), pts(i,1)) = scores(i);
end



end

