function varargout = depthviewer(varargin)
% DEPTHVIEWER MATLAB code for depthviewer.fig
%      DEPTHVIEWER, by itself, creates a new DEPTHVIEWER or raises the existing
%      singleton*.
%
%      H = DEPTHVIEWER returns the handle to a new DEPTHVIEWER or the handle to
%      the existing singleton*.
%
%      DEPTHVIEWER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in DEPTHVIEWER.M with the given input arguments.
%
%      DEPTHVIEWER('Property','Value',...) creates a new DEPTHVIEWER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before depthviewer_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to depthviewer_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help depthviewer

% Last Modified by GUIDE v2.5 21-Apr-2014 13:16:34

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @depthviewer_OpeningFcn, ...
                   'gui_OutputFcn',  @depthviewer_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT
end

% --- Executes just before depthviewer is made visible.
function depthviewer_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to depthviewer (see VARARGIN)

% Choose default command line output for depthviewer
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes depthviewer wait for user response (see UIRESUME)
% uiwait(handles.figure1);
end

% --- Outputs from this function are returned to the command line.
function varargout = depthviewer_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

end


% --- Executes on slider movement.
function depth_slider_Callback(hObject, eventdata, handles)
% hObject    handle to depth_slider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider
cur_slider_val = get(hObject, 'Value');
set(handles.slider_val_label, 'String', num2str(cur_slider_val));

% show depth in seleted level
low_range = max(cur_slider_val-0.04, 0);
high_range = min(cur_slider_val+0.04, 1);
set(handles.depth_range_label, 'String', ['Depth Range: ' num2str(low_range) ' - ' num2str(high_range)]);

low_depth = im2bw(handles.depthImg, low_range);
high_depth = im2bw(handles.depthImg, high_range);
sel_depth = (low_depth ~= high_depth);
imshow(sel_depth, 'Parent', handles.depth_th_axis);
colormap jet
colorbar

end

% --- Executes during object creation, after setting all properties.
function depth_slider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to depth_slider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

end

% --------------------------------------------------------------------
function op_menu_Callback(hObject, eventdata, handles)
% hObject    handle to op_menu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% reset slider
set(handles.depth_slider, 'Enable', 'On');
set(handles.depth_slider, 'Value', 0);
set(handles.slider_val_label, 'String', '0');

datapath = 'E:\Datasets\objectness\pos\';
[filename, ~] = uigetfile([datapath '\*.jpg'], 'Color Data');

% color image
cimgpath = [datapath filename];
handles.colorImg = imread(cimgpath);
% show
imshow(handles.colorImg, 'Parent', handles.color_axis);
% colormap jet
% colorbar

% compute color edge map
grayimg = rgb2gray(handles.colorImg);
[handles.colorEdgeMap, ~] = imgradient(grayimg, 'sobel');
handles.colorEdgeMap = getnormimg(handles.colorEdgeMap);
% show
imshow(handles.colorEdgeMap, [], 'Parent', handles.color_edge_map);
% colormap jet
% colorbar

% depth map
[~, fname, ~] = fileparts(filename);
dmappath = [datapath fname '_d.txt'];
dmap = load(dmappath);
dmap = double(dmap);
handles.depthImg = getnormimg(dmap);
computeNormalForDepthmap(dmap);
% show
imshow(handles.depthImg, 'Parent', handles.depth_axis);
% colormap jet
% colorbar

% depth edge map
[handles.depthEdgeMap, ~] = imgradient(dmap, 'sobel');
handles.depthEdgeMap = getnormimg(handles.depthEdgeMap);
% show
imshow(handles.depthEdgeMap, [], 'Parent', handles.depth_edge_map);
% colormap jet
% colorbar

% combined edge map
handles.fusedEdgeMap = sqrt(handles.colorEdgeMap.^2 + handles.depthEdgeMap.^2);
handles.fusedEdgeMap = getnormimg(handles.fusedEdgeMap);
imshow(handles.fusedEdgeMap, [], 'Parent', handles.fused_edge_map);
colormap jet
colorbar

% init label image
labelimg = ones(size(dmap));
imshow(labelimg, 'Parent', handles.depth_th_axis);
colormap jet
colorbar

% set data
% handles.mindepth = min(min(handles.depthImg));
% handles.maxdepth = max(max(handles.depthImg));
% update
guidata(hObject, handles);

while 1
    rect = getrect(handles.fused_edge_map);
    % show normed gradient patch
    smallimg = imcrop(handles.fusedEdgeMap, rect);
    smallimg = imresize(smallimg, [8 8]);
    smallimg = imresize(smallimg, [64, 64]);
    smallimg = smallimg ./ max(smallimg(:));
    h = figure(1);
    imshow(smallimg)
    hold on
    pause
    close(h)
end

end

% reconstruct 3d point cloud from depth and compute normal vector for each
% point
function norm3d = computeNormalForDepthmap(depthimg)
% create 3d point cloud
% invF = [594.21 0 320/594.21; 0 591.04 240/594.21; 0 0 1];
% 
% % homogeneous coordinates
% [rows, cols] = find(depthimg>=0);
% dvals = depthimg(sub2ind(size(depthimg), rows, cols));
% homo_coord = [cols'; rows'; dvals'];
% dvalmap = [dvals'; dvals'; dvals'];
% homo_coord = homo_coord ./ dvalmap;
% 
% % get local coordinates
% local_coord = invF \ homo_coord;
% local_coord = local_coord .* dvalmap;

[pcl, ~] = depthToCloud(depthimg);

SavePointCloudPLY('samp.ply', pcl);

% show 3d
% figure;
% scatter3(local_coord(1,1:50:end), local_coord(2,1:50:end), local_coord(3,1:50:end)); 
% hold on

% compute normal
%norm3d = normnd(local_coord');

end

function SavePointCloudPLY(savefile, model_data)

% output to ply file for rendering
fn = savefile;
fp = fopen(fn, 'w');
fprintf(fp, 'ply\nformat ascii 1.0\nelement vertex %d\nproperty float32 x\nproperty float32 y\nproperty float32 z\n', size(model_data, 1)*size(model_data, 2));
fprintf(fp, 'end_header\n\n');
for i = 1:size(model_data, 1)
    for j=1:size(model_data, 2)
        fprintf(fp, '%f %f %f\n', model_data(i,j,1:3));
    end
end
fclose(fp);

end


function normimg = getnormimg(img)

normimg = (img-min(img(:))) ./ (max(img(:))-min(img(:)));

end
