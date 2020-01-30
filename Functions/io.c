#include "../Headers/io.h"
#include "../Headers/utility.h"

Commit ConstructBranch(char* path) { //path je put do direktorija ukljucujuci i njega .git/.commits/####
	Commit temp = NULL; FILE* fp = NULL;
	char pathBuffer[BUFFER_SIZE] = { 0 };
	char parseBuffer[BUFFER_SIZE] = { 0 };
	char* token = NULL;
	char* entryToken = NULL;

	sprintf(pathBuffer, "%s/%s", path, ".commit");
	fp = fopen(pathBuffer, "r");
	while (fgets(parseBuffer, BUFFER_SIZE, fp)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		token = strrchr(parseBuffer, ' ') + 1;
		entryToken = strtok(parseBuffer, ":");
		if (!_strcmpi(entryToken, "Parent Commit")) {
			if (_strcmpi(token, "NULL")) {
				temp = AllocateCommit();
					strcpy(temp->commitPath, path);
					path = strrchr(path, '/') + 1;
					sscanf(path, "%d", &(temp->commitID));
					token[strcspn(token, "\n")] = '\0';
					temp->parentCommit = ConstructBranch(token);
			}
			else {
				temp = AllocateCommit();
				path = strrchr(path, '/') + 1;
				sscanf(path, "%d", &(temp->commitID));
				temp->parentCommit = NULL;
			}
		}
		else if (!_strcmpi(entryToken, "Branch")) {
			strcpy(temp->branchName, token);
		}
	}

	fclose(fp);
	return temp;
}

Commit ConstructCommitTree(char* path, Head heads) {
	Commit temp = NULL, commonAncestor = NULL;
	FILE* fp = NULL; Head firstHead = heads;
	char pathBuffer[BUFFER_SIZE] = { 0 };
	char parseBuffer[BUFFER_SIZE] = { 0 };
	char* token = NULL;
	char* entryToken = NULL;
	int errnum = 0;

	sprintf(pathBuffer, "%s/%s", path, ".commit");
	fp = fopen(pathBuffer, "r");
	while (fgets(parseBuffer, BUFFER_SIZE, fp)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		token = strrchr(parseBuffer, ' ') + 1;
		entryToken = strtok(parseBuffer, ":");

		if(!_strcmpi(entryToken, "Foreign"))
			continue;
		
		if (!_strcmpi(entryToken, "Parent Commit")) {
			if (_strcmpi(token, "NULL")) {
				temp = AllocateCommit();
				if (temp == NULL)
					return NULL;

				strcpy(temp->commitPath, path);
				path = strrchr(path, '/') + 1;
				sscanf(path, "%d", &(temp->commitID));
				token[strcspn(token, "\n")] = '\0';
				while (heads != NULL && heads->commitPointer!=NULL) {
					commonAncestor = CheckPathway(heads->commitPointer, token);
					if (commonAncestor != NULL) {
						commonAncestor->childrenNumber++;
						temp->parentCommit = commonAncestor;
						fgets(parseBuffer, BUFFER_SIZE, fp);
						parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
						token = strrchr(parseBuffer, ' ') + 1;
						strcpy(temp->branchName, token);
						fclose(fp);
						return temp;					
					}
					heads = heads->nextHead;					
				}
				temp->fileTree = CreateFolderNode(temp->commitPath);
				if (temp->fileTree == NULL)
					return NULL;

				errnum=ConstructFileTree(temp->fileTree, temp->commitPath);
				if (errnum != RETURN_OK)
					return NULL;

				temp->parentCommit = ConstructCommitTree(token, firstHead);
				if (temp == NULL)
					return NULL;

				temp->parentCommit->childrenNumber++;
			}
			else {
				temp = AllocateCommit();
				if (temp == NULL)
					return NULL;

				strcpy(temp->commitPath, path);
				path = strrchr(path, '/') + 1;
				sscanf(path, "%d", &(temp->commitID));
				temp->parentCommit = NULL;
			}
		}
		else if (!_strcmpi(entryToken, "Branch")) {
			strcpy(temp->branchName, token);
		}
	}

	fclose(fp);
	return temp;
}

int ConstructFileTree(FolderNode parentFolder, char* passedPath)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	wchar_t sPath[BUFFER_SIZE];
	wchar_t sDir[BUFFER_SIZE];
	FolderNode tempFolder = NULL;
	FileNode tempFile = NULL;
	char pathBuffer[BUFFER_SIZE];
	int errnum = 0;

	mbstowcs(sDir, passedPath, BUFFER_SIZE);
	wsprintf(sPath, L"%s/*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return RETURN_NOEXIST;
	}

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") !=0 && wcscmp(fdFile.cFileName, L".commit") != 0 && wcscmp(fdFile.cFileName, L".git")!=0)
		{
			wsprintf(sPath, L"%s/%s", sDir, fdFile.cFileName);
			wcstombs(pathBuffer, sPath, BUFFER_SIZE);

			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{	
				tempFolder = CreateFolderNode(pathBuffer);
				if (tempFolder == NULL)
					return RETURN_ERROR_MEM_ALLOC;

				InsertChild(parentFolder, tempFolder);
				errnum=ConstructFileTree(tempFolder, pathBuffer);
			}
			else {
				tempFile = CreateFileNode(pathBuffer);
				if (tempFile == NULL)
					return RETURN_ERROR_MEM_ALLOC;
				AppendFile(parentFolder, tempFile);
			}
		}
	} while (FindNextFile(hFind, &fdFile));

	FindClose(hFind);
	return RETURN_OK;
}