#include "common.h"

#include "os_comm.h"
#include "os_fs.h"
#include "os_shm.h"

#include <string>


#ifdef WIN32

void *GetShm(int key, size_t size, bool forceCreate)
{
	char filePath[255];
	sprintf(filePath, "%sshm%d", GetModuleFileDirectory().c_str(), key);
	char name[255];
	sprintf(name, "shm%d", key);
#ifdef UNICODE
	int a = 0;
	a++;
	a--;
#endif

	HANDLE hFile = CreateFileA(	filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if (INVALID_HANDLE_VALUE == hFile && !forceCreate)
	{
		return NULL;
	}

	hFile = CreateFileA(	filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL,
		NULL );
	if (INVALID_HANDLE_VALUE == hFile)
	{ 
		printf("\nopen key file (%s) failed", name);
		assert__(false);
		return NULL;
	}

	HANDLE hFileMap;
	hFileMap = OpenFileMappingA(PAGE_READWRITE|FILE_MAP_READ | FILE_MAP_WRITE, false, name);
	if (NULL != hFileMap)
	{
		printf("\nattach key success = \"%s\"", name);
	}
	else
	{
		int ret = GetLastError();
		unsigned int maxSizeHigh = (unsigned int)((size >> 32) & 0xFFFFFFFF);
		unsigned int maxSizeLow = (unsigned int)(size & 0xFFFFFFFF);
		hFileMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, maxSizeHigh, maxSizeLow, name);
		if (NULL == hFileMap)
		{
			CloseHandle(hFile);
			assert__(false);
			return NULL;
		}
		printf("\ncreate key success = \"%s\"", name);
	}

	void *shm = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, NULL);
	if (NULL == shm)
	{
		CloseHandle(hFile);
		CloseHandle(hFileMap);
		printf("\nmap key(%s) failed", name);
		assert__(false);
		return NULL;
	}
	return shm; 
}


#else

#include <sys/ipc.h>
#include <sys/shm.h>

void *GetShm_(int key, size_t size, int flag)
{
	int shmId;
	void *shm;
	if ((shmId = shmget(key, size, flag)) == -1)
		return NULL;
	shm = shmat(shmId, NULL, 0);
	if ((void *)-1 == shm)
		return NULL;
	return shm;
}

void *GetShm(int key, size_t size, bool forceCreate)
{
	void *shm;
	shm = GetShm_(key, size, 0666);
	if (NULL == shm && !forceCreate) 
		return shm;

	shm = GetShm_(key, size, 0666 | IPC_CREAT);
	return shm;
}

#endif