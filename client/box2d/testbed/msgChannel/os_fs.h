#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <vector>

#include "os_type.h"

#include <string>

using namespace std;

string GetFullPath(const string &path);
string GetFullPath(const string &rootPath, const string &path);

bool IsRelativePath(const string &path);
string GetRootPath();
void GetDirFiles(std::vector<std::string> &filepaths, const string &dirpath);
string PathCat(const string &path1, const string &path2);

int GetFileSize(FILE *fp);

void SetDefaultRootPath(const string &path);
string GetDefaultRootPath();

string GetModuleFileDirectory();

extern string g_defaultRootPath;