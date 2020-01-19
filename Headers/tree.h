#ifndef HEADER_TREE_
#define HEADER_TREE_

#include "common.h"

#define FILESTATE_COMMITTED 97
#define FILESTATE_MODIFIED 98
#define FILESTATE_STAGED 99

typedef struct _fileNode {

	char* filePath;
	char* fileHash;
	int fileState;
	struct _fileNode* nextFile;

}_fileNode; typedef _fileNode* FileNode;

typedef struct _folderNode {

	char* folderPath;
	FileNode fileTree;
	struct _folderNode* nextFolder;

}_folderNode; typedef _folderNode* FolderNode;



unsigned long Hash(const char*);
#endif

