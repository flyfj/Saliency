//////////////////////////////////////////////////////////////////////////
//	simple implementation of non-maximum suppression
//	fengjie@MSRA.VC
//	2010-7-27
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <algorithm>
using namespace std;

template<class ScoreRectType>
vector<ScoreRectType> nms(vector<ScoreRectType>& input, float overlapRate)
{
	int wincount = 0;
	vector<bool> flags(input.size(), false);

	// sort windows by scores (low->high)
	sort(input.begin(), input.end(), ScoredRect::comp_by_score);

	for(size_t i=0; i<input.size(); i++)
	{
		for(size_t j=i+1; j<input.size(); j++)
		{
			int xx1 = max(input[i].X, input[j].X);
			int yy1 = max(input[i].Y, input[j].Y);
			int xx2 = min(input[i].GetRight(), input[j].GetRight());
			int yy2 = min(input[i].GetBottom(), input[j].GetBottom());
			float width = xx2-xx1+1;
			float height = yy2-yy1+1;
			if(width>0 && height>0)
			{
				float overlap = (width*height)/(input[i].Width*input[i].Height+input[j].Width*input[j].Height-width*height);
				if(overlap > overlapRate)
				{
					flags[i] = true;	//suppressed
					wincount++;
					break;
				}
			}
		}
	}

	vector<ScoredRect> res;
	res.reserve(wincount);
	for(int i=input.size()-1; i>=0; i--)
	{
		if(!flags[i])
			res.push_back(input[i]);
	}

	return res;
}