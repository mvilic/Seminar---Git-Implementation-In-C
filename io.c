#include "io.h"
#include "commit.h"

int ConstructTreeFromFiles(TreeNode fileTree, char* folderPath) {

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];
	wchar_t sDir[2048];
	char* pathBuffer[2048];

	swprintf(sDir, L"%hs", folderPath); 
	wsprintf(sPath, L"%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return RETURN_NOEXIST;
	}

	do
	{ 
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
		{ 
			wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

			//Is the entity a File or Folder? 
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wprintf(L"Directory: %s\n", sPath);
				ListDirectoryContents(sPath);
			}
			else {
				wctomb(pathBuffer, sPath);
				strcpy(fileTree->filePath, pathBuffer);
				wprintf(L"File: %s\n", sPath);
			}
		}
	} while (FindNextFile(hFind, &fdFile));

	FindClose(hFind);

	return RETURN_OK;
}

Commit ConstructCommitTree(char* path) {

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	FILE* fp = NULL;
	wchar_t sPath[2048];
	wchar_t sDir[2048];
	char pathBuffer[2048] = { 0 };
	char parseBuffer[2048] = { 0 };
	char* token = NULL;
	Commit temp = NULL;

	wsprintf(sPath, L"%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return RETURN_NOEXIST;
	}

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
		{
			wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

			//Is the entity a File or Folder? 
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wprintf(L"Directory: %s\n", sPath);
				wctomb(pathBuffer, sPath);
				sprintf(pathBuffer, "%s\\%s", pathBuffer, ".commit");
				fp = fopen(pathBuffer, "r");
				while (fgets(parseBuffer, 2048, fp)) {
					if (!_strcmpi(parseBuffer, "Parent Commit: NULL")) {
						token = strchr(parseBuffer, " ") + 1;
						temp = AllocateCommit();
						wctomb(temp->commitID, sPath);
						temp->parentCommit = ConstructCommitTree(token);
						
					}

				}
			}
		}
	} while (FindNextFile(hFind, &fdFile));

	FindClose(hFind);
	fclose(fp);

	return temp;
}

Commit FindParent(char* path) { //path je put do direktorija ukljucujuci i njega .git/.commits/####
	Commit temp = NULL; FILE* fp = NULL;
	char pathBuffer[2048] = { 0 };
	char parseBuffer[2048] = { 0 };
	char* token = NULL;

	sprintf(pathBuffer, "%s\\%s", path, ".commit");
	fp = fopen(pathBuffer, "r");
	while (fgets(parseBuffer, 2048, fp)) {
		if (_strcmpi(parseBuffer, "Parent Commit: NULL")) {
			token = strrchr(parseBuffer, ' ') + 1;
			temp = AllocateCommit();
			strcpy(temp->commitPath, path);
			path = strrchr(path, '\\') +1;
			sscanf(path, "%d", &(temp->commitID));
			temp->parentCommit = FindParent(token);
		}
		else if (!_strcmpi(parseBuffer, "Parent Commit: NULL")) {
			temp = AllocateCommit();
			path = strrchr(path, '\\')+1;
			sscanf(path, "%d", &(temp->commitID));
			temp->parentCommit = NULL;
		}
	
	}

	fclose(fp);
	return temp;
}