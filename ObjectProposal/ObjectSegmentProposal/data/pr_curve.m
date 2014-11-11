




data = load('curve.mat');
data = data.curve;
len = size(data, 2);

figure
xlabel('Window Number')
ylabel('Recall')
hold on
ylim([0 1])
hold on
grid on
plot(1:len, data, 'r-')
hold on
legend('Geodesic Proposal')