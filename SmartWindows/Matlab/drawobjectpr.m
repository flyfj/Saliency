

objpr = load('nyu2_objpr.txt');
salpr = load('nyu2_salpr.txt');
depthpr = load('nyu2_dccpr.txt');
% dvarpr = load('nyu2_dvarpr.txt');

figure
plot(objpr(:,2), objpr(:,1), 'g-')
hold on
plot(salpr(:,2), salpr(:,1), 'r-')
hold on
plot(depthpr(:,2), depthpr(:,1), 'b-')
hold on
% plot(dvarpr(:,2), dvarpr(:,1), 'c-')
% hold on
legend('objectness', 'color cc', 'depth cc', 'depth var')