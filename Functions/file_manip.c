#include "../Headers/file_manip.h"
#include "../Headers/utility.h"

/*
###################################################
#				General Functions				  #
###################################################
*/

//Clear contents of passed directory
int ClearDirectory(char* passedPath)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	char pathBuffer[BUFFER_SIZE];
	wchar_t sPath[BUFFER_SIZE];
	wchar_t sDir[BUFFER_SIZE];

	//Multibyte to wide string conversion. Windows requirement.
	mbstowcs(sDir, passedPath, BUFFER_SIZE);
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
			//Multibyte to wide string conversion. Windows requirement.
			wsprintf(sPath, L"%s/%s", sDir, fdFile.cFileName);
			wcstombs(pathBuffer, sPath, BUFFER_SIZE);

			//Found a folder. Enter it in a recursive case. Delete the directory after it's contents are deleted
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				ClearDirectory(pathBuffer);
				RemoveDirectoryA(pathBuffer);
			}
			//Found a file, delete it
			else {
				DeleteFileA(pathBuffer);
			}
		}
	} while (FindNextFile(hFind, &fdFile));

	//Close file handle
	FindClose(hFind);
	return RETURN_OK;
}

/*
###################################################
#					Commiting					  #
###################################################
*/

//Create commit file tree in the commits' folder that aren't foreign references
//The passed folder tree holds files that need to be committed, but their path still holds the active directory
int CreateCommitOnDisk(FolderNode folderTree, int replacePosition, char* commitPath) { //folder tree i commit path su oboje od novonastalog commita, ali u folder treeu su pathovi jos uvik sa active_direcotry
	FolderNode currentFolder = NULL;
	char* pathToFolder = NULL;
	currentFolder = folderTree;
	char buf[BUFFER_SIZE];
	int errnum = 0;

	//Iterate through the file tree of commit to push
	while (currentFolder != NULL) {
		//Extract path to folder relative to the commit root folder
		pathToFolder = currentFolder->folderPath + replacePosition;
		//If current folder node has a child, create that folder and recurse further into it
		if (currentFolder->firstChild != NULL) {
			//If we're dealing with with the root directory of the commit
			if (!strlen(pathToFolder)) {
				strcpy(currentFolder->folderPath, commitPath);
				strcpy(currentFolder->folderName, (strrchr(commitPath, '/') + 1));
				_mkdir(currentFolder->folderPath);
			}
			//If we're dealing with a regular folder
			else {
				//Construct actual folder path in respect to the new commit  and create folder
				snprintf(buf, BUFFER_SIZE, "%s%s", commitPath, pathToFolder);
				strcpy(currentFolder->folderPath, buf);
				_mkdir(currentFolder->folderPath);
			}
			//Create files within the current folder node in the actual folder in the commits' directory
			errnum=CommitFiles(currentFolder->fileList, currentFolder->folderPath);
			if (errnum != RETURN_OK)
				return errnum;
			//Recurse into the current folder's first child
			errnum=CreateCommitOnDisk(currentFolder->firstChild, replacePosition, commitPath);
			if (errnum != RETURN_OK)
				return errnum;
		}
		//Current folder does not, in fact, have any children
		else {
			snprintf(buf, BUFFER_SIZE, "%s%s", commitPath, pathToFolder);
			strcpy(currentFolder->folderPath, buf);
			_mkdir(currentFolder->folderPath);
			errnum=CommitFiles(currentFolder->fileList, currentFolder->folderPath);
			if (errnum != RETURN_OK)
				return errnum;
		}

		currentFolder = currentFolder->nextSibling;
	}

	return RETURN_OK;
}

//Copy files that aren't foreign references
int CommitFiles(FileNode fileList, char* folderPath) {
	FileNode currentFile = fileList;
	char fileToCopy[BUFFER_SIZE], parseBuffer[BUFFER_SIZE];
	char* tempHash=NULL;
	FILE* sourceFile = NULL, * destinationFile = NULL;
	size_t size = 0;

	//Iterate through the passed file list
	while (currentFile != NULL) {
		//Copy over into the new commit folder files that have changes from it's parent
		if (currentFile->foreignFlag == 0) {
			strcpy(fileToCopy, currentFile->filePath);
			snprintf(currentFile->filePath, BUFFER_SIZE, "%s/%s", folderPath, currentFile->fileName);

			sourceFile = fopen(fileToCopy, "r");
			if (sourceFile == NULL) {
				printf("\nError opening source file: [%s]!\n", fileToCopy);
				return RETURN_WARNING_FILE_OPEN;
			}

			destinationFile = fopen(currentFile->filePath, "w");
			if (destinationFile == NULL) {
				printf("\nError creating destination file: [%s]\n", currentFile->filePath);
				fclose(sourceFile);
				return RETURN_WARNING_FILE_OPEN;
			}

			while (size = fread(parseBuffer, 1, BUFFER_SIZE, sourceFile))
				fwrite(parseBuffer, 1, size, destinationFile);

			fclose(sourceFile);
			fclose(destinationFile);

			currentFile->fileState = FILESTATE_COMMITTED;
			tempHash = hash(currentFile->filePath);
			strcpy(currentFile->fileHash, tempHash);
			free(tempHash);
		}

		currentFile = currentFile->nextFile;
	}

	return RETURN_OK;
}

/*
###################################################
#					Checking Out				  #
###################################################

*/
//Create folder tree of given commit in the active directory
//Follows very similar logic to the commit creation methods
int CheckoutCommit(FolderNode folderTree, int replacePosition, char* activeDirectoryPath) {
	FolderNode currentFolder = NULL;
	char* pathToFolder = NULL;
	currentFolder = folderTree;
	char buf[BUFFER_SIZE];
	int errnum = 0;

	while (currentFolder != NULL) {

		pathToFolder = currentFolder->folderPath + replacePosition;
		if (currentFolder->firstChild != NULL) {
			if (!strlen(pathToFolder)) {
				strcpy(buf, activeDirectoryPath);
				_mkdir(activeDirectoryPath);
			}
			else {
				snprintf(buf, BUFFER_SIZE, "%s%s", activeDirectoryPath, pathToFolder);
				_mkdir(buf);
			}
			
			errnum=CheckoutFiles(currentFolder->fileList, buf);
			if (errnum != RETURN_OK)
				return errnum;
			
			CheckoutCommit(currentFolder->firstChild, replacePosition, activeDirectoryPath);
		}
		else {
			snprintf(buf, BUFFER_SIZE, "%s%s", activeDirectoryPath, pathToFolder);
			_mkdir(buf);
			errnum=CheckoutFiles(currentFolder->fileList, buf);
			if (errnum != RETURN_OK)
				return errnum;
		}

		currentFolder = currentFolder->nextSibling;
	}

	return RETURN_OK;
}

//Copy the passed file list into the passed folder
//Logic identical to file commit method, but disregards foreign status
int CheckoutFiles(FileNode fileList, char* folderPath) {
	FileNode currentFile = fileList;
	char fileToCopy[BUFFER_SIZE], fileToCreate[BUFFER_SIZE], parseBuffer[BUFFER_SIZE];
	FILE* sourceFile = NULL, * destinationFile = NULL;
	size_t size = 0;

	while (currentFile != NULL) {
	
		strcpy(fileToCopy, currentFile->filePath);
		snprintf(fileToCreate, BUFFER_SIZE, "%s/%s", folderPath, currentFile->fileName);

		sourceFile = fopen(fileToCopy, "r");
		if (sourceFile == NULL) {
			printf("\nError opening source file: [%s]!\n", fileToCopy);
			return RETURN_WARNING_FILE_OPEN;
		}

		destinationFile = fopen(fileToCreate, "w");
		if (destinationFile == NULL) {
			fclose(sourceFile);
			printf("\nError creating destination file: [%s]\n", currentFile->filePath);
			return RETURN_WARNING_FILE_OPEN;
		}

		while (size = fread(parseBuffer, 1, BUFFER_SIZE, sourceFile))
			fwrite(parseBuffer, 1, size, destinationFile);

		fclose(sourceFile);
		fclose(destinationFile);

		currentFile = currentFile->nextFile;
	}

	return RETURN_OK;
}