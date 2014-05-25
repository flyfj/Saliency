function [] = compFeatMaps( cimg, dimg )
%COMPFEATMAPS Summary of this function goes here
%   compute feataure maps

% color gradient
cgrad = compGrad2(cimg);
cgrad = getnormimg(cgrad);

% depth gradient
dgrad = compGrad2(dimg);
dgrad = getnormimg(dgrad);

% normal gradient
ngrad = compNormalMap(dimg);
ngrad = getnormimg(ngrad);

% final map
bmap = (cgrad + dgrad + ngrad) ./ 3;

% display
figure
subplot(1, 4, 1)
imshow(cgrad, [])
colormap jet
colorbar
hold on
subplot(1, 4, 2)
imshow(dgrad, [])
colormap jet
colorbar
hold on
subplot(1, 4, 3)
imshow(ngrad, [])
colormap jet
colorbar
hold on
subplot(1, 4, 4)
imshow(bmap, [])
colormap jet
colorbar
hold on

end

