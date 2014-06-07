#include "ObjectTester.h"


ObjectTester::ObjectTester(void)
{
}

//////////////////////////////////////////////////////////////////////////

void ObjectTester::TestObjectRanking(const DatasetName& dbname)
{
	// load dataset
	FileInfos img_fns;
	FileInfos dmap_fns;

	if(dbname == DB_NYU2_RGBD)
	{
		NYUDepth2DataMan nyudata;
		nyudata.GetImageList(img_fns);
		nyudata.GetDepthmapList(dmap_fns);
		map<string, vector<ImgWin>> gtwins;
		nyudata.LoadGTWins(img_fns, gtwins);
	}
	



	GenericObjectDetector obj_det;

}