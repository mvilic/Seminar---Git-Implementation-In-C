#include "common.h"
#include "tree.h"
#include "commit.h"

Commit CreateCommit(TreeNode passedFileTree, Commit passedParentCommit) {
	char buffer[BUFFER_SIZE] = { 0 };
	Commit temp = AllocateCommit();

	GetLocalTime(&(temp->commitDate));
	temp->commitID = temp->commitDate.wYear + temp->commitDate.wMonth + temp->commitDate.wDay + temp->commitDate.wHour + temp->commitDate.wMinute + temp->commitDate.wSecond;
	temp->fileTree = passedFileTree;
	temp->parentCommit = passedParentCommit;

	sprintf(buffer, "sample_repo\\.git\\commits\\%d", temp->commitID);

	strcpy(temp->commitPath, buffer);

	return temp;
}

Commit AllocateCommit() {
	Commit temp = NULL; char buffer[BUFFER_SIZE] = { 0 };
	temp = (Commit)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	return temp;
}