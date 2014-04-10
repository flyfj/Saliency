


#pragma once

#include <vector>
#include "common.h"


// evaluate performance of object proposal
class WindowEvaluator
{
private:

	float ComputeWinMatchScore(const ImgWin& qwin, const ImgWin& gwin);

public:

	string gtdir;

	WindowEvaluator(void);

	// best matched detection windows with ground truth
	bool FindBestWins(const vector<ImgWin>& det_wins, const vector<ImgWin>& gt_wins, vector<ImgWin>& bestWins);

};

