#include "file_manip.h"
#include "utility.h"


int RemoveDirectoryFull(const wchar_t* sDir)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	char pathBuffer[BUFFER_SIZE];
	wchar_t sPath[BUFFER_SIZE];

	//file mask. *.* = everything
	wsprintf(sPath, L"%s/*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return RETURN_NOEXIST;
	}

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
		{
			wsprintf(sPath, L"%s/%s", sDir, fdFile.cFileName);
			wcstombs(pathBuffer, sPath, BUFFER_SIZE);

			//File or Folder? 
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				RemoveDirectoryFull(sPath);
				RemoveDirectoryA(pathBuffer);
			}
			else {
				DeleteFileA(pathBuffer);
			}
		}
	} while (FindNextFile(hFind, &fdFile));

	FindClose(hFind);
	return RETURN_OK;
}