#include "file_manip.h"


int DirectoryExists(char* path) {
	struct stat info;

	if (stat(path, &info) != 0)
		return RETURN_NOEXIST;
	else if (info.st_mode & S_IFDIR)
		return RETURN_EXIST;
	else
		return RETURN_NOTDIR;
}

bool ListDirectoryContents(const wchar_t* sDir)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];

	//Specify a file mask. *.* = We want everything! 
	wsprintf(sPath, L"%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return RETURN_NOEXIST;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories. 
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
		{
			//Build up our file path using the passed in 
			//  [sDir] and the file/foldername we just found: 
			wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

			//Is the entity a File or Folder? 
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wprintf(L"Directory: %s\n", sPath);
				ListDirectoryContents(sPath);  
			}
			else {
				wprintf(L"File: %s\n", sPath);
			}
		}
	} while (FindNextFile(hFind, &fdFile)); 

	FindClose(hFind); 

	return RETURN_OK;
}

unsigned int Hash(const char* str)
{
	int key = 3;
	unsigned int hash = 0;
	FILE* file = NULL;
	char buffer[BUFFER_SIZE] = { 0 };

	file = fopen(str, "r");
	if (file == NULL)
		return RETURN_WARNING_FILE_OPEN;

	while (!feof(file)) {
		hash = hash * key + getc(file);
	}

	fclose(file);
	return hash;
}