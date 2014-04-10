#include "WindowEvaluator.h"


WindowEvaluator::WindowEvaluator(void)
{
}

//////////////////////////////////////////////////////////////////////////

float WindowEvaluator::ComputeWinMatchScore(const ImgWin& qwin, const ImgWin& gwin)
{
	Rect box1(qwin.x, qwin.y, qwin.width, qwin.height);
	Rect box2(gwin.x, gwin.y, gwin.width, gwin.height);

	Rect interBox = box1 & box2;
	Rect unionBox = box1 | box2;

	if(unionBox.area() > 0)
		return (float)interBox.area() / unionBox.area();
	else
		return 0;
}

bool WindowEvaluator::FindBestWins(const vector<ImgWin>& det_wins, const vector<ImgWin>& gt_wins, vector<ImgWin>& bestWins)
{
	bestWins.clear();

	for(size_t i=0; i<gt_wins.size(); i++)
	{
		// find best matched detection window (>0.5)
		float bestscore = 0;
		ImgWin bestwin;
		for(size_t j=0; j<det_wins.size(); j++)
		{
			float s = ComputeWinMatchScore(gt_wins[i], det_wins[j]);
			if(s > bestscore)
			{
				bestscore = s;
				bestwin = det_wins[j];
			}
		}

		if(bestscore > 0.5)
			bestWins.push_back(bestwin);
	}

	return true;
}