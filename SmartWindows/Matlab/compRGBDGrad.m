function gradimg = compRGBDGrad( cimg, dimg )
%COMPRGBDGRAD Summary of this function goes here
%   Detailed explanation goes here


grayimg = rgb2gray(cimg);
[cgrad, ~] = imgradient(grayimg, 'sobel');
%cgrad = getnormimg(cgrad);

% dimg = double(dimg);
% [dgrad, ~] = imgradient(dimg, 'sobel');
% dgrad = getnormimg(dgrad);
% imshow(dgrad)
% pause

% combine
gradimg = cgrad;  %sqrt(cgrad.^2 + dgrad.^2);

end

function normimg = getnormimg(img)

normimg = (img-min(img(:))) ./ (max(img(:))-min(img(:)));

end