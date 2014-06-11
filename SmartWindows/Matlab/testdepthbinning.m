function testdepthbinning(dmap)

% visualize depth histogram
binnum = 20;
binstep = 255 / binnum;

figure
imshow(dmap,[])
colormap jet
colorbar

ndmap = getnormimg(dmap);
ndmap = ndmap * 255;
[imgh, imgw] = size(dmap);

dcode = zeros(imgh, imgw);

for r=1:imgh
    for c=1:imgw
        binid = ndmap(r, c) / binstep;
        binid = int32(binid);
        dcode(r,c) = binid;
    end
end

figure
imshow(dcode, [])
colormap jet
colorbar

