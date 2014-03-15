


#pragma once

#include <vector>
#include "common.h"


class WindowEvaluator
{
private:

	ImgWin LoadBox(FileNode& fn);

public:

	string gtdir;

	WindowEvaluator(void);

	bool LoadVOC07Boxes(const vector<std::string>& imgnames, std::map<string, vector<ImgWin>>& gtwins);

	//////////////////////////////////////////////////////////////////////////
	// database analysis
	//////////////////////////////////////////////////////////////////////////
	// 
	bool GenerateWinSamps(const vector<string>& imgnames);
};

