#include "common.h"
#include "os_fs.h"
#include "os_comm.h"

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

string g_defaultRootPath;


void SetDefaultRootPath(const string &path)
{ 
	g_defaultRootPath = path; 
}

string GetDefaultRootPath()
{ 
	return g_defaultRootPath; 
}

string GetFullPath(const string &path)
{
	if (!IsRelativePath(path))
	{
		return path;
	}
	return PathCat(GetDefaultRootPath(), path);
}

string GetFullPath(const string &rootPath, const string &path)
{
	if (!IsRelativePath(path))
	{
		return path;
	}
	return PathCat(rootPath, path);
}

bool IsRelativePath(const string &path)
{
#ifdef WIN32
	return (path.size() >= 2 && path[1] != ':');
#else\


	return (path.size() >= 1 && path[0] != '/');
#endif

	return true;
}

int GetFileSize(FILE *fp)
{
	int pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return size;
}


string PathCat(const string &path1, const string &path2)
{
	if (0 == path1.length())
	{
		return path2;
	}
	string p1;
	string p2;

	if ((path1.length() >= 1) &&  ('/' == path1[path1.length() - 1]))
		p1 = path1.substr(0, path1.length() - 1);
	else
		p1 = path1;

	if ((path2.length() >= 1) && ('/' == path2[0]))
		p2 = path2.substr(1, path2.length() - 1);
	else 
		p2 = path2;

	return p1 + "/" + p2;
}

#ifdef WIN32

string GetModuleFileDirectory()
{
	char path[260];

	GetModuleFileNameA(NULL, path, 260);
	char *p = strrchr(path, '\\');
	if (p)
		*(p + 1) = '\0';

	return path;
}

void RecursiveGetDirFiles(std::vector<std::string> &filepaths, const string &dirpath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	string path;

	string dirExp = PathCat(dirpath, "*");  // c:/script/quest/*


	hFind = FindFirstFile(dirExp.c_str(), &FindFileData);

	if(hFind == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		if((strcmp(FindFileData.cFileName,".") == 0)
			|| (strcmp(FindFileData.cFileName,"..") == 0))
		{
			continue;
		}

		path = PathCat(dirpath, FindFileData.cFileName);   

		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			RecursiveGetDirFiles(filepaths, path);   
		else
			filepaths.push_back(path);

	}while(FindNextFile(hFind, &FindFileData));

	::FindClose(hFind);
}

#else

string GetModuleFileDirectory()
{
	return "";
}

void RecursiveGetDirFiles(std::vector<std::string> &filepaths, const string &dirpath)
{
	DIR *dir = opendir(dirpath.c_str());   
	if (NULL == dir) return;

	struct dirent *s_dir;   
	while ((s_dir = readdir(dir)) != NULL)   
	{   
		if ((strcmp(s_dir->d_name, ".") == 0)
			||(strcmp(s_dir->d_name, "..") == 0))   
			continue;   

		string path = PathCat(dirpath, s_dir->d_name);

		struct stat file_stat;   
		stat(path.c_str(), &file_stat);   
		if (S_ISDIR(file_stat.st_mode))   
			RecursiveGetDirFiles(filepaths, path);
		else
			filepaths.push_back(path);
	}   
	closedir(dir);  
}

#endif


void GetDirFiles(std::vector<std::string> &filepaths, const string &dirpath)
{
	filepaths.clear();
	RecursiveGetDirFiles(filepaths, dirpath);
}


