


#pragma once

#include <vector>
#include "common.h"


class WindowEvaluator
{

public:
	WindowEvaluator(void);

	bool LoadVOC07Boxes(const std::string& filename, std::vector<ImgWin>& boxes);
};

