#ifndef HEADER_TREE_
#define HEADER_TREE_

#include "common.h"

#define FILESTATE_COMMITTED 97
#define FILESTATE_MODIFIED 98
#define FILESTATE_STAGED 99

typedef struct _treeNode {

	char* filePath;
	unsigned int fileHash;
	int fileState;
	struct _treenode* nextFile;

}_treeNode; typedef _treeNode* TreeNode;

TreeNode CreateTreeNode(char*);

#endif

