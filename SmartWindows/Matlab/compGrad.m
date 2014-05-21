function grad = compGrad( img )
%COMPGRAD Summary of this function goes here
%   Detailed explanation goes here

sz = size(img);
if(length(sz) == 3)
    grayimg = rgb2gray(img);
else
    grayimg = img;
end
h = sz(1);
w = sz(2);
Ix = zeros(h, w);
Iy = zeros(h, w);

% Ix
% left and right border
Ix(:, 1) = abs(grayimg(:,2) - grayimg(:,1)) .* 2;
Ix(:, w) = abs(grayimg(:,w-1) - grayimg(:,w)) .* 2;
% inside gradient
for i=2:w-1
    Ix(:, i) = abs(grayimg(:,i+1)-grayimg(:,i-1));
end

% Iy
% top and bottom
Iy(1,:) = abs(grayimg(2,:) - grayimg(1,:)) .* 2;
Iy(h,:) = abs(grayimg(h-1,:) - grayimg(h,:)) .* 2;
% inside
for i=2:h-1
    Iy(i, :) = abs(grayimg(i+1,:) - grayimg(i-1,:));
end

grad = Ix + Iy;
% grad = uint8(grad);


end

