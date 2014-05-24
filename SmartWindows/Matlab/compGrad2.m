function gradmag = compGrad2( inputimg )
%COMPGRAD Summary of this function goes here
%   Detailed explanation goes here
%  use matlab normal gradient function

verbose = 1;

sz = size(inputimg);
if length(sz) == 3
    grayimg = rgb2gray(inputimg);
else
    grayimg = inputimg;
end

[gradmag, graddir] = imgradient(grayimg, 'sobel');

if verbose == 1
    
figure
subplot(1, 3, 1)
imshow(inputimg,[]), title('input image')
hold on
subplot(1, 3, 2)
imshow(gradmag, []), title('Gradient magnitude')
hold on
subplot(1, 3, 3)
imshow(graddir, []), title('Gradient direction')
hold on

colormap jet
colorbar

end


end

