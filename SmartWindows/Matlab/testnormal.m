
% test surface normal estimation
% input a set of 3d points, estimate normal for all them and visualize

pts = [0 0 0; 10 0 0; 0 10 0; 10 10 0; 5 5 5; 10 15 20 ];

% plot
figure
scatter3(pts(:,1), pts(:,2), pts(:,3))
hold on

% compute normal
coeff = pca(pts);
normal = coeff(:, end);
normal = normal .* 10;

plot3([0 normal(1)], [0, normal(2)], [0 normal(3)], 'r-')



