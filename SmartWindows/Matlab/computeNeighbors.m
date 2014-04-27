function mesh = computeNeighbors(config, mesh)
% Compute the indices of the neighbors for each vertex, and store them in a
% cell array of mesh.neighbors field.

if ~exist('id', 'var'); id = ''; end;
if ~isfield(config, 'k'); config.k = 5; end

mesh.neighbors = computeNeighborsMex(mesh.v, config.k);

% Compute the normal vectors if the model file doesn't provide it
if ~isfield(mesh, 'n') || isempty(mesh.n)
    normals = zeros(3, mesh.vn);
    parfor i = 1: mesh.vn
        pts = mesh.v(:, mesh.neighbors{i});
        pts = bsxfun(@minus, pts, mean(pts, 2));
        coeff = pca(pts');
        normals(:, i) = coeff(:, end);
        if dot(normals(:, i), mesh.v(:, 1)) < 0
            normals(:, i) = -normals(:, i);
        end
    end
    mesh.n = normals;
else
    % fix the [0, 0, 0] normals
    normals = zeros(3, mesh.vn);
    toCompute = find(sum(abs(mesh.n))<=1e-6);
    for i = toCompute
        pts = mesh.v(:, mesh.neighbors{i});
        pts = bsxfun(@minus, pts, mean(pts, 2));
        coeff = pca(pts');
        normals(:, i) = coeff(:, end);
        if dot(normals(:, i), mesh.v(:, 1)) < 0
            normals(:, i) = -normals(:, i);
        end
    end
    mesh.n(:, toCompute) = normals(:, toCompute);
end
end