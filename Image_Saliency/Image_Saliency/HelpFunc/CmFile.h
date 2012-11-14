#pragma once

struct CmFile
{
	static inline string GetFolder(const string& path);
	static inline string GetName(const string& path);
	static inline string GetNameNE(const string& path);

	// Get file names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
	static int GetNames(const string & nameW, vecS & names);
	static int GetNames(const string &nameW, vecS &names, string &dir);
	static int GetNamesNE(const string& nameWC, vecS &names);
	static int GetNamesNE(const string& nameWC, vecS &names, string &dir);

	static BOOL MkDir(const string&  path);
	static Mat LoadMask(const string& fileName);

	inline static BOOL Copy(const string &src, const string &dst, BOOL failIfExist = FALSE);
};

/************************************************************************/
/* Implementation of inline functions                                   */
/************************************************************************/
string CmFile::GetFolder(const string& path)
{
	return path.substr(0, path.find_last_of("\\/")+1);
}

string CmFile::GetName(const string& path)
{
	int start = path.find_last_of("\\/")+1;
	int end = path.find_last_not_of(' ')+1;
	return path.substr(start, end - start);
}

string CmFile::GetNameNE(const string& path)
{
	int start = path.find_last_of("\\/")+1;
	int end = path.find_last_of('.');
	if (end >= 0)
		return path.substr(start, end - start);
	else
		return path.substr(start,  path.find_last_not_of(' ')+1 - start);
}

BOOL CmFile::Copy(const string &src, const string &dst, BOOL failIfExist)
{
	return ::CopyFileA(src.c_str(), dst.c_str(), failIfExist);
}
