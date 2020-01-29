#include "../Headers/file_manip.h"
#include "../Headers/utility.h"

int ClearDirectory(char* passedPath)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	char pathBuffer[BUFFER_SIZE];
	wchar_t sPath[BUFFER_SIZE];
	wchar_t sDir[BUFFER_SIZE];

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
			wsprintf(sPath, L"%s/%s", sDir, fdFile.cFileName);
			wcstombs(pathBuffer, sPath, BUFFER_SIZE);

			//File or Folder? 
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				ClearDirectory(pathBuffer);
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

int CreateCommitOnDisk(FolderNode folderTree, int replacePosition, char* commitPath) { //folder tree i commit path su oboje od novonastalog commita, ali u folder treeu su pathovi jos uvik sa active_direcotry
	FolderNode currentFolder = NULL;
	char* pathToFolder = NULL;
	currentFolder = folderTree;
	char buf[BUFFER_SIZE];

	while (currentFolder != NULL) {
		
		pathToFolder = currentFolder->folderPath + replacePosition;
		if (currentFolder->firstChild != NULL) {
			if (!strlen(pathToFolder)) {
				strcpy(currentFolder->folderPath, commitPath);
				strcpy(currentFolder->folderName, (strrchr(commitPath, '/') + 1));
				_mkdir(currentFolder->folderPath);
			}
			else {
				snprintf(buf, BUFFER_SIZE, "%s%s", commitPath, pathToFolder);
				strcpy(currentFolder->folderPath, buf);
				_mkdir(currentFolder->folderPath);
			}
			CommitFiles(currentFolder->fileList, currentFolder->folderPath);
			CreateCommitOnDisk(currentFolder->firstChild, replacePosition, commitPath);
		}
		else {
			snprintf(buf, BUFFER_SIZE, "%s%s", commitPath, pathToFolder);
			strcpy(currentFolder->folderPath, buf);
			_mkdir(currentFolder->folderPath);
			CommitFiles(currentFolder->fileList, currentFolder->folderPath);
		}

		currentFolder = currentFolder->nextSibling;
	}

	return RETURN_OK;
}

int CommitFiles(FileNode fileList, char* folderPath) {
	FileNode currentFile = fileList;
	char fileToCopy[BUFFER_SIZE], parseBuffer[BUFFER_SIZE];
	char* tempHash=NULL;
	FILE* sourceFile = NULL, * destinationFile = NULL;
	size_t size = 0;

	while (currentFile != NULL) {
		
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

int CheckoutCommit(FolderNode folderTree, int replacePosition, char* activeDirectoryPath) {
	FolderNode currentFolder = NULL;
	char* pathToFolder = NULL;
	currentFolder = folderTree;
	char buf[BUFFER_SIZE];

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
			CheckoutFiles(currentFolder->fileList, buf);
			CheckoutCommit(currentFolder->firstChild, replacePosition, activeDirectoryPath);
		}
		else {
			snprintf(buf, BUFFER_SIZE, "%s%s", activeDirectoryPath, pathToFolder);
			_mkdir(buf);
			CheckoutFiles(currentFolder->fileList, buf);
		}

		currentFolder = currentFolder->nextSibling;
	}

	return RETURN_OK;
}

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