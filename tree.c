#include "common.h"
#include "tree.h"

TreeNode CreateTreeNode(char* path) {
	TreeNode temp = NULL;
	temp = (TreeNode)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	strcpy(temp->filePath, path);
	temp->fileHash = Hash(path);
	temp->fileState = FILESTATE_COMMITTED;
	temp->nextFile = NULL;

	return temp;
}