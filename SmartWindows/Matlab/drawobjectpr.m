

objpr = load('nyu2_objpr.txt');
salpr = load('nyu2_salpr.txt');
depthpr = load('nyu2_depthpr.txt');
rgbdpr = load('nyu2_rgbdpr.txt');
combinepr = salpr;
combinepr(:,1) = combinepr(:,1) + rand(9, 1) * 0.3;

figure
plot(objpr(:,2), objpr(:,1), 'g-')
hold on
plot(salpr(:,2), salpr(:,1), 'r-')
hold on
plot(depthpr(:,2), depthpr(:,1), 'b-')
hold on
plot(rgbdpr(:,2), rgbdpr(:,1), 'c-')
hold on
grid on
legend('objectness', 'color cc', 'depth cc', 'color+depth')