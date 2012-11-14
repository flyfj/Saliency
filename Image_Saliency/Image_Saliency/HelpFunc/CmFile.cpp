#include "../StdAfx.h"

BOOL CmFile::MkDir(const string&  _path)
{
	const char* path = _path.c_str();
	static char buffer[1024];
	strcpy(buffer, path);
	for (int i = 0; buffer[i] != 0; i ++)
	{
		if (buffer[i] == '\\')
		{
			buffer[i] = '\0';
			CreateDirectoryA(buffer, 0);
			buffer[i] = '\\';
		}
	}
	return CreateDirectoryA(path, 0);
}


// Get image names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
int CmFile::GetNames(const string& nameWC, vector<string>& names)
{
	names.clear();
	WIN32_FIND_DATA fileFindData;
	HANDLE hFind = ::FindFirstFile(nameWC.c_str(), &fileFindData);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do 
	{
		if (fileFindData.cFileName[0] == '.')
			continue; // filter the '..' and '.' in the path
		if (fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue; // Ignore sub-folders
		names.push_back(fileFindData.cFileName);
	} while (::FindNextFile(hFind, &fileFindData));
	FindClose(hFind);
	return (int)names.size();
}

int CmFile::GetNames(const string &nameW, vecS &names, string &dir)
{
	dir = GetFolder(nameW);
	return GetNames(nameW, names);
}

int CmFile::GetNamesNE(const string& nameWC, vector<string> &names, string &dir)
{
	int fNum = GetNames(nameWC, names, dir);
	for (int i = 0; i < fNum; i++)
		names[i] = GetNameNE(names[i]);
	return fNum;
}

int CmFile::GetNamesNE(const string& nameWC, vecS &names)
{
	string dir;
	return GetNamesNE(nameWC, names, dir);
}

Mat CmFile::LoadMask(const string& fileName)
{
	Mat mask = imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);
	CV_Assert(mask.data != NULL);
	compare(mask, 128, mask, CV_CMP_GT);
	return mask;
}
