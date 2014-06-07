

objpr = load('objpr.txt');
salpr = load('salpr.txt');

figure
plot(objpr(:,2), objpr(:,1), 'g-')
hold on
plot(salpr(:,2), salpr(:,1), 'r-')
hold on
legend('objectness', 'saliency')
pause
close all