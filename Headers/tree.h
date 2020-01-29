#ifndef HEADER_TREE_
#define HEADER_TREE_

#include "common.h"

/*
###################################################
#			Constant Definitions				  #
###################################################
*/

#define FILESTATE_COMMITTED 97
#define FILESTATE_STAGED 98

/*
###################################################
#			Structure Definitions				  #
###################################################
*/

typedef struct _fileNode {

	char fileName[BUFFER_SIZE];
	char filePath[BUFFER_SIZE];
	char fileHash[33];
	char foreignFlag;
	int fileState;
	struct _fileNode* nextFile;

}_fileNode; typedef _fileNode* FileNode;

typedef struct _folderNode {

	char folderName[BUFFER_SIZE];
	char folderPath[BUFFER_SIZE];
	FileNode fileList;
	struct _folderNode* nextSibling;
	struct _folderNode* firstChild;
	
}_folderNode; typedef _folderNode* FolderNode;

/*
###################################################
#				Memory Management				  #
###################################################
*/

FolderNode CreateFolderNode(char* folderPath); FileNode CreateFileNode(char* filePath);
int DeallocateFolderNode(FolderNode toDeallocate); int DeallocateFileList(FileNode toDeallocate);

/*
###################################################
#				General Functions				  #
###################################################
*/

int InsertChild(FolderNode targetFolder, FolderNode folderToInsert);
int AppendFile(FolderNode targetFolder, FileNode fileToInsert);
FileNode FindFile(FolderNode targetFileTree, char* targetFile);

/*
###################################################
#				Staging Area					  #
###################################################
*/

int CheckFilestate(FolderNode targetFileTree, int* result);
int StageForCommit(FolderNode, char*, int, FolderNode); int StageForBranch(FolderNode parentFolder);

#endif
