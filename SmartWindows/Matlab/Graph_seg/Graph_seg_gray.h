
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GRAPH_SEG_GRAY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GRAPH_SEG_GRAY_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef GRAPH_SEG_GRAY_EXPORTS
#define GRAPH_SEG_GRAY_API __declspec(dllexport)
#else
#define GRAPH_SEG_GRAY_API __declspec(dllimport)
#endif

// This class is exported from the Graph_seg_gray.dll
class GRAPH_SEG_GRAY_API CGraph_seg_gray {
public:
	CGraph_seg_gray(void);
	// TODO: add your methods here.
};

extern GRAPH_SEG_GRAY_API int nGraph_seg_gray;

GRAPH_SEG_GRAY_API int fnGraph_seg_gray(void);

