function SavePointCloudPLY(savefile, model_data)

% output to ply file for rendering
fn = savefile;
fp = fopen(fn, 'w');
fprintf(fp, 'ply\nformat ascii 1.0\nelement vertex %d\nproperty float32 x\nproperty float32 y\nproperty float32 z\n', size(model_data,2));
fprintf(fp, 'end_header\n\n');
for j=1:size(model_data, 2)
    fprintf(fp, '%f %f %f\n', model_data(:,j));
end
fclose(fp);

end