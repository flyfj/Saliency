function [cgrad, dgrad, ngrad] = compFeatMaps( cimg, dimg, useImgFMT )
%COMPFEATMAPS Summary of this function goes here
%   compute feataure maps

% color gradient
cgrad = compGrad2(cimg);

% depth gradient
dgrad = compGrad2(dimg);

% normal gradient
ngrad = compNormalMap(dimg);

% convert to image format
if useImgFMT
    cgrad = uint8( getnormimg(cgrad) .* 255 );
    dgrad = uint8( getnormimg(dgrad) .* 255 );
    ngrad = uint8( getnormimg(ngrad) .* 255 );
end
    
% final map
% bmap = (cgrad + dgrad + ngrad) ./ 3;

% display
% figure
% subplot(1, 4, 1)
% imshow(cgrad, [])
% colormap jet
% colorbar
% hold on
% subplot(1, 4, 2)
% imshow(dgrad, [])
% colormap jet
% colorbar
% hold on
% subplot(1, 4, 3)
% imshow(ngrad, [])
% colormap jet
% colorbar
% hold on
% subplot(1, 4, 4)
% imshow(bmap, [])
% colormap jet
% colorbar
% hold on

end

