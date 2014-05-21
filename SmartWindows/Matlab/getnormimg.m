function normimg = getnormimg( img )
%GETNORMIMG Summary of this function goes here
%   Detailed explanation goes here

img = double(img);
normimg = (img-min(img(:))) ./ (max(img(:))-min(img(:)));

end

