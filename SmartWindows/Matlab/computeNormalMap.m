function nmap = computeNormalMap( dmap )
%COMPUTENORMALMAP Summary of this function goes here
%   Detailed explanation goes here

[imgh, imgw] = size(dmap);

[pcl, ~] = depthToCloud(dmap);
mesh.v = pcl;
mesh.vn = size(pcl, 2);

config.k = 10;
mesh = computeNeighbors(config, mesh);
mesh.n = -mesh.n;

ns = zeros(imgh, imgw, 3);
cnt = 1;
for i=1:imgh
    for j=1:imgw
        ns(i, j, :) = mesh.n(:, cnt);
        cnt = cnt + 1;
    end
end

nmapx = ns(:,:,1);
nmapy = ns(:,:,2);
nmapz = ns(:,:,3);

nmapx = compGrad(nmapx);
nmapy = compGrad(nmapy);
nmapz = compGrad(nmapz);

nmap = (nmapx + nmapy + nmapz);


end

