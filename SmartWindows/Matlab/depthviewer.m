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

% Last Modified by GUIDE v2.5 20-Apr-2014 19:44:18

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


% --- Outputs from this function are returned to the command line.
function varargout = depthviewer_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


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


% --- Executes during object creation, after setting all properties.
function depth_slider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to depth_slider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
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

[filename, pathname] = uigetfile('E:\Datasets\RGBD_Dataset\Berkeley\VOCB3DO\KinectColor\*.png; *.jpg; *.txt', 'Color Data');


% color image
cimgpath = ['E:\Datasets\RGBD_Dataset\Berkeley\VOCB3DO\KinectColor\' filename];
handles.colorImg = imread(cimgpath);
% show
imshow(handles.colorImg, 'Parent', handles.color_axis);
colormap jet
colorbar

% depth map
[~, fname, ~] = fileparts(filename) 
dmappath = ['E:\Datasets\RGBD_Dataset\Berkeley\VOCB3DO\RegisteredDepthData\' fname '_abs_smooth.png'];
dmap = imread(dmappath);
dmap = im2double(dmap);
handles.depthImg = (dmap-min(dmap(:))) ./ (max(dmap(:))-min(dmap(:)))
% show
imshow(handles.depthImg, 'Parent', handles.depth_axis);
colormap jet
colorbar
% show 3d depth
% figure
% surf(handles.depthImg, 'FaceColor', 'texturemap');


labelimg = ones(size(dmap));
imshow(labelimg, 'Parent', handles.depth_th_axis);
colormap jet
colorbar

% set data
% handles.mindepth = min(min(handles.depthImg));
% handles.maxdepth = max(max(handles.depthImg));
guidata(hObject, handles);  % update
