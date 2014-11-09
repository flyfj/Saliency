
%data = load('sp_pred_19.txt');
data = load('curve.mat');
data = data.curve;
sz = size(data, 2);
len = min(5, sz);

figure
for i=1:len
    subplot(1, len, i)
    axis([0,1,0,1])
    plot(1:size(data,2), data(i,:), 'r-')
    hold on
end

