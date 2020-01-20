#include "tree.h"

FileNode CreateFileNode(char* path) {
	FileNode temp = NULL;
	temp = (FileNode)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	strcpy(temp->filePath, path);
	temp->fileHash = hash(path);
	temp->fileState = FILESTATE_COMMITTED;
	temp->nextFile = NULL;

	return temp;
}

FolderNode CreateFolderNode(char* path) {
	FolderNode temp = NULL;
	temp = (FolderNode)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	strcpy(temp->folderPath, path);
	temp->fileList = NULL;
	temp->nextSibling = NULL;
	temp->firstChild = NULL;

	return temp;
}

int InsertChild(FolderNode parentFolder, FolderNode toInsert) {
	FolderNode firstChild = parentFolder->firstChild; //da bi se moglo setati po listi dijece i siblinga bez utjecanja na originalnu strukturu

	if (parentFolder->firstChild == NULL)
		parentFolder->firstChild = toInsert;
	else {
		while (firstChild->nextSibling != NULL)
			firstChild = firstChild->nextSibling;

		firstChild->nextSibling = toInsert;
	}

	return RETURN_OK;
}

int AppendFile(FolderNode folder, FileNode fileToInsert) {
	FileNode firstFile = folder->fileList;

	if (folder->fileList == NULL) {
		folder->fileList = fileToInsert;
		return RETURN_OK;
	}
	else {
		while (firstFile->nextFile != NULL)
			firstFile = firstFile->nextFile;

		firstFile->nextFile = fileToInsert;
	}

	return RETURN_OK;
}

unsigned long Hash(const char* str)
{
	int key = 3;
	unsigned long hash = 0;
	FILE* file = NULL;
	char buffer[BUFFER_SIZE] = { 0 }, c = 0;

	file = fopen(str, "r");
	if (file == NULL)
		return RETURN_WARNING_FILE_OPEN;

	while ((c=getc(file))!=EOF) {
		hash = (hash + key) + (hash-c);
	}

	fclose(file);
	return hash;
}
