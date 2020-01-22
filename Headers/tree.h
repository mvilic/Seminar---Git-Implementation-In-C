#ifndef HEADER_TREE_
#define HEADER_TREE_

#include "common.h"

#define FILESTATE_COMMITTED 97
#define FILESTATE_MODIFIED 98
#define FILESTATE_STAGED 99

typedef struct _fileNode {

	char filePath[BUFFER_SIZE];
	char fileHash[33];
	int fileState;
	struct _fileNode* nextFile;

}_fileNode; typedef _fileNode* FileNode;

typedef struct _folderNode {

	char folderPath[BUFFER_SIZE];
	FileNode fileList;
	struct _folderNode* nextSibling;
	struct _folderNode* firstChild;

}_folderNode; typedef _folderNode* FolderNode;

FolderNode CreateFolderNode(char*); FileNode CreateFileNode(char*);
int InsertFileNode(FolderNode, FileNode); int InsertFolderNode(FolderNode, FolderNode);
int InsertChild(FolderNode, FolderNode); int AppendFile(FolderNode, FileNode);


#endif

