function varargout = imgpicker(varargin)
% IMGPICKER MATLAB code for imgpicker.fig
%      IMGPICKER, by itself, creates a new IMGPICKER or raises the existing
%      singleton*.
%
%      H = IMGPICKER returns the handle to a new IMGPICKER or the handle to
%      the existing singleton*.
%
%      IMGPICKER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in IMGPICKER.M with the given input arguments.
%
%      IMGPICKER('Property','Value',...) creates a new IMGPICKER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before imgpicker_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to imgpicker_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help imgpicker

% Last Modified by GUIDE v2.5 24-May-2014 18:15:52

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @imgpicker_OpeningFcn, ...
                   'gui_OutputFcn',  @imgpicker_OutputFcn, ...
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


% --- Executes just before imgpicker is made visible.
function imgpicker_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to imgpicker (see VARARGIN)

% Choose default command line output for imgpicker
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes imgpicker wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = imgpicker_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --------------------------------------------------------------------
function open_menu_Callback(hObject, eventdata, handles)
% hObject    handle to open_menu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

datapath = 'E:\Datasets\objectness\b3d_pos\';
[filename, ~] = uigetfile([datapath '\*.jpg'], 'Color Data');
[~, fname, ~] = fileparts(filename);

cimg = imread([datapath fname '.jpg']);
dimg = load([datapath fname '_d.txt']);

compFeatMaps(cimg, dimg);


