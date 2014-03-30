

#include "Tools.h"


void ToolFactory::GetFilesFromDir(const string& dir, const string& type, FileInfos& fileInfos)
{
	fileInfos.clear();
	FileInfo fileinfo;

	struct _finddata_t ffblk;
	int done = 1;
	string filelist = dir + type;	// TODO: is there a way to find multiple ext files without doing several times?
	int handle = _findfirst(filelist.c_str(), &ffblk);
	if(handle != -1)
	{
		while( done != -1 )
		{
			string fname(ffblk.name);
			//fname = fname.substr(0, fname.length()-4);	// image name without extension
			string path = dir + fname;
			fileinfo.filename = fname;
			fileinfo.filepath = path;
			fileInfos.push_back(fileinfo);

			done = _findnext(handle, &ffblk);
		}
	}

}

void ToolFactory::GetDirsFromDir(const string& dir, DirInfos& dirInfos, const string& outputFile)
{
	dirInfos.clear();
	DirInfo dirInfo;

	struct _finddata_t ffblk;
	int done = 1;
	string files = dir + "*.*";	// TODO: is there a way to find multiple ext files without doing several times?
	int handle = _findfirst(files.c_str(), &ffblk);
	if(handle != -1)
	{
		while( done != -1 )
		{
			// _A_SUBDIR is not right
			if(ffblk.attrib == 2064)
			{
				string fname(ffblk.name);
				if(fname != "." && fname != "..")
				{
					dirInfo.dirname = fname;
					dirInfo.dirpath = dir + fname + "\\";
					dirInfos.push_back(dirInfo);
				}
			}
				
			done = _findnext(handle, &ffblk);
		}
	}

	if(!dirInfos.empty())
	{
		ofstream out(outputFile.c_str());
		if(out.is_open())
		{
			for(size_t i=0; i<dirInfos.size(); i++)
				out<<dirInfos[i].dirname<<endl;
		}
			
	}
}

void ToolFactory::RemoveEmptyDir(const string& dir, const string& type)
{
	DirInfos dir_infos;
	GetDirsFromDir(dir, dir_infos);
	for(size_t i=0; i<dir_infos.size(); i++)
	{
		FileInfos info;
		GetFilesFromDir(dir_infos[i].dirpath, type, info);
		if(info.empty())
		{
			_rmdir(dir_infos[i].dirpath.c_str());
			cout<<"Remove "<<dir_infos[i].dirpath<<endl;
		}

		cout<<"Finish "<<dir_infos[i].dirpath<<endl;
	}
}
