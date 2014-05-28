function nmap = compNormalMap( dimg )
%COMPNORMALMAP Summary of this function goes here

addpath('obj_toolbox');

[imgh, imgw] = size(dimg);
% project depth to 3d
dimg = double(dimg);
[pcl, ~] = depthToCloud(dimg);
mesh.v = pcl';
mesh.f = zeros(1, 3);
write_ply(mesh.v, [], 'temp.ply');

% call meshlabserver to extract normal
res = system('c:\meshlab\meshlabserver -i temp.ply -o temp_n.ply -s normal_script.mlx -om vn')
if res ~= 0
    disp('Error extracting normals with meshlab.');
    nmap = [];
    return;
end

% read in obj
[newmesh, ~] = read_ply('temp_n.ply');
normals = [newmesh.vertex.nx newmesh.vertex.ny newmesh.vertex.nz];

% convert to 2d image
nx = reshape(normals(:,1), [imgw imgh]);
nx = nx';
nx = compGrad2(nx);
ny = reshape(normals(:,2), [imgw imgh]);
ny = ny';
ny = compGrad2(ny);
nz = reshape(normals(:,3), [imgw imgh]);
nz = nz';
nz = compGrad2(nz);
nmap = nx + ny + nz;%sqrt(nx.^2 + ny.^2 + nz.^2);
% filter noise
nmap = medfilt2(nmap);

% figure
% subplot(1, 3, 1)
% imshow(nx,[])
% hold on
% subplot(1, 3, 2)
% imshow(ny, [])
% hold on
% subplot(1, 3, 3)
% imshow(nz, [])
% hold on
% 
% figure
% imshow(nmap, [])
% hold on
% colormap jet
% colorbar

% pause 
% close all

end

