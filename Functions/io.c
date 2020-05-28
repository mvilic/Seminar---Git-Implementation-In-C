#include "../Headers/io.h"
#include "../Headers/utility.h"

//Constructs single branch from head commit entry in index file
Commit ConstructBranch(char* path) {
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

//Construct complete tree for given head commit
Commit ConstructCommitTree(char* path, Head heads) {
	Commit temp = NULL, commonAncestor = NULL;
	FILE* fp = NULL; Head firstHead = heads;
	char pathBuffer[BUFFER_SIZE] = { 0 };
	char parseBuffer[BUFFER_SIZE] = { 0 };
	char* token = NULL;
	char* entryToken = NULL;
	int errnum = 0;

	//Build path to commit config file
	sprintf(pathBuffer, "%s/%s", path, ".commit");
	fp = fopen(pathBuffer, "r");
	if (fp == NULL)
		return NULL;

	//Iterate through the entire file, parsing entry type
	while (fgets(parseBuffer, BUFFER_SIZE, fp)) {
		//Remove newline character from read entry
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		//Separate entry value from entry type
		token = strrchr(parseBuffer, ' ') + 1;
		//Remove ":" from entry type 
		entryToken = strtok(parseBuffer, ":");

		if(!_strcmpi(entryToken, "Foreign"))
			continue;
		
		//Parse entry type
		if (!_strcmpi(entryToken, "Parent Commit")) {
			//Check if listed parent commit is NULL. If not, continue recursion
			if (_strcmpi(token, "NULL")) {
				temp = AllocateCommit();
				if (temp == NULL)
					return NULL;

				//Paste read commit path into the commit node
				strcpy(temp->commitPath, path);
				//Get commitID from the commit path
				path = strrchr(path, '/') + 1;
				sscanf(path, "%d", &(temp->commitID));
				//Take care of any rogue newline characters
				token[strcspn(token, "\n")] = '\0';
				//Check if the listed parent commit already exists in the instanced commit tree.
				//If yes, point the current node's parent pointer to it and exit recursion
				while (heads != NULL && heads->commitPointer!=NULL) {
					commonAncestor = CheckPathway(heads->commitPointer, token);
					if (commonAncestor != NULL) {
						commonAncestor->childrenNumber++;
						temp->parentCommit = commonAncestor;
						//Read branch name now since the current recursive case exits before the branch reading phase
						fgets(parseBuffer, BUFFER_SIZE, fp);
						parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
						token = strrchr(parseBuffer, ' ') + 1;
						strcpy(temp->branchName, token);
						fclose(fp);
						return temp;					
					}
					heads = heads->nextHead;					
				}
				//Allocate first folder node of the commit's file tree
				temp->fileTree = CreateFolderNode(temp->commitPath);
				if (temp->fileTree == NULL)
					return NULL;
				//Construct the commit's file tree with the previously allocated node as the root
				errnum=ConstructFileTree(temp->fileTree, temp->commitPath);
				if (errnum != RETURN_OK)
					return NULL;
				//Call next recursive case
				temp->parentCommit = ConstructCommitTree(token, firstHead);
				if (temp == NULL)
					return NULL;
				//After the recursive case is done, increment parents' children count by this recursive instance
				temp->parentCommit->childrenNumber++;
			}
			else {
				//Found the root commit of the repository. Instantiate and and recursion.
				temp = AllocateCommit();
				if (temp == NULL)
					return NULL;

				strcpy(temp->commitPath, path);
				path = strrchr(path, '/') + 1;
				sscanf(path, "%d", &(temp->commitID));
				temp->parentCommit = NULL;
			}
		}
		//Self-explanatory
		else if (!_strcmpi(entryToken, "Branch")) {
			strcpy(temp->branchName, token);
		}
	}
	//Also self-explanatory
	fclose(fp);
	return temp;
}

//Instantiates and constructs the passed commits' file tree using the passed folder node as the root folder
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

	//Conversion between multibyte and wide string types, a windows platform requirement
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
			//More multibyte<->wide string conversions
			wsprintf(sPath, L"%s/%s", sDir, fdFile.cFileName);
			wcstombs(pathBuffer, sPath, BUFFER_SIZE);

			//File in question is a directory. Instantiate the node, insert it into the parent folder as a child and enter it in a recursive case
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{	
				tempFolder = CreateFolderNode(pathBuffer);
				if (tempFolder == NULL)
					return RETURN_ERROR_MEM_ALLOC;

				InsertChild(parentFolder, tempFolder);
				errnum=ConstructFileTree(tempFolder, pathBuffer);
			}
			//File in question is a file. Instantiate and append into the current folders' file list.
			else {
				tempFile = CreateFileNode(pathBuffer);
				if (tempFile == NULL)
					return RETURN_ERROR_MEM_ALLOC;
				AppendFile(parentFolder, tempFile);
			}
		}
	} while (FindNextFile(hFind, &fdFile));

	//Close file handle
	FindClose(hFind);
	return RETURN_OK;
}