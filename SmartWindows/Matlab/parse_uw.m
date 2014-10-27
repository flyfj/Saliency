root_dir = 'E:\Datasets\RGBD_Dataset\UW\rgbd-scene-dataset1\';
gt = load([root_dir 'kitchen_small/kitchen_small_1.mat']);
% each image, generate a ground truth
gt = gt.bboxes;

for i=1:length(gt)
    curfn = [root_dir 'kitchen_small/gt1/' num2str(i) '.txt'];
    fid = fopen(curfn, 'w');
    data = gt(i);
    if(isempty(data{1}))
        continue;
    end
    fprintf(fid, '%d\n', length(data{1}));
    for j=1:length(data{1})
        % xmin, ymin, xmax, ymax
        fprintf(fid, '%d %d %d %d\n', data{1}(j).left, data{1}(j).top, data{1}(j).right, data{1}(j).bottom);
    end
    fclose(fid);
    disp(num2str(i));
end
