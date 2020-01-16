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

unsigned int Hash(const char* str)
{
	int key = 3;
	unsigned int hash = 0;
	FILE* file = NULL;
	char buffer[BUFFER_SIZE] = { 0 };

	file = fopen(str, "r");
	if (file == NULL)
		return RETURN_WARNING_FILE_OPEN;

	while (!feof(file)) {
		hash = hash * key + getc(file);
	}

	fclose(file);
	return hash;
}