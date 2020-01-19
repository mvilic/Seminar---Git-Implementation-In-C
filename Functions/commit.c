#include "common.h"
#include "tree.h"
#include "commit.h"

Commit AllocateCommit() {
	Commit temp = (Commit)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	temp->parentCommit = NULL;
	temp->fileTree = NULL;

	return temp;
}

int DeallocateCommit(Commit commit) {

}


Head AllocateHead() {
	Head temp = (Head)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	temp->commitPointer = NULL;
	temp->nextHead = NULL;

	return temp;
}

Commit CreateCommit(FolderNode passedFileTree, Commit passedParentCommit) {
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

int GetHeads(Head heads, char* gitDir) {
	FILE* headsFile = NULL; Head headsStart = heads;
	char parseBuffer[2048];
	char indexPath[2048];

	sprintf(indexPath, "%s\\%s", gitDir, ".heads");
	headsFile = fopen(indexPath, "r");
	while (fgets(parseBuffer, BUFFER_SIZE, headsFile)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		heads->commitPointer = ConstructCommitTree(parseBuffer, headsStart);
		heads->nextHead = AllocateHead();
		heads = heads->nextHead;
	}

	fclose(headsFile);

	return RETURN_OK;
}

Commit CommonAncestor(Commit commit1, Commit commit2) {
	Commit commit2Start = NULL;

	if (commit1 == NULL || commit2 == NULL)
		return NULL;

	commit2Start = commit2;
	while (commit1->parentCommit != NULL) {
		while (commit2->parentCommit != NULL); {
			if (_strcmpi(commit1->commitPath, commit2->commitPath) == 0)
				return commit1;
			else
				commit2=commit2->parentCommit;
		
		}
		
		commit2 = commit2Start;
		commit1 = commit1->parentCommit;
	}

	return NULL;
};

Commit CheckPathway(Commit commit1, char* parentPath) {
	Commit commit2Start = NULL;

	if (commit1 == NULL)
		return NULL;

	
	while (commit1->parentCommit != NULL) {
			if (_strcmpi(commit1->commitPath, parentPath) == 0)
				return commit1;
			else
				commit1 = commit1->parentCommit;
	}

	return NULL;
};