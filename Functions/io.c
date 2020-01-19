#include "io.h"

Commit ConstructBranch(char* path) { //path je put do direktorija ukljucujuci i njega .git/.commits/####
	Commit temp = NULL; FILE* fp = NULL;
	char pathBuffer[BUFFER_SIZE] = { 0 };
	char parseBuffer[BUFFER_SIZE] = { 0 };
	char* token = NULL;
	char* entryToken = NULL;

	sprintf(pathBuffer, "%s\\%s", path, ".commit");
	fp = fopen(pathBuffer, "r");
	while (fgets(parseBuffer, 2048, fp)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		token = strrchr(parseBuffer, ' ') + 1;
		entryToken = strtok(parseBuffer, ":");
		if (!_strcmpi(entryToken, "Parent Commit")) {
			if (_strcmpi(token, "NULL")) {
				temp = AllocateCommit();
					strcpy(temp->commitPath, path);
					path = strrchr(path, '\\') + 1;
					sscanf(path, "%d", &(temp->commitID));
					token[strcspn(token, "\n")] = '\0';
					temp->parentCommit = ConstructBranch(token);
			}
			else {
				temp = AllocateCommit();
				path = strrchr(path, '\\') + 1;
				sscanf(path, "%d", &(temp->commitID));
				temp->parentCommit = NULL;
			}
		
		}
		else if (!_strcmpi(entryToken, "Branch")) {
			strcpy(temp->branchName, token);
		}
	}

	fclose(fp);
	return temp;
}

Commit ConstructCommitTree(char* path, Head heads) { //path je put do direktorija ukljucujuci i njega: .git/.commits/####
	Commit temp = NULL, commonAncestor = NULL;
	FILE* fp = NULL; Head firstHead = heads;
	char pathBuffer[BUFFER_SIZE] = { 0 };
	char parseBuffer[BUFFER_SIZE] = { 0 };
	char* token = NULL;
	char* entryToken = NULL;

	sprintf(pathBuffer, "%s\\%s", path, ".commit");
	fp = fopen(pathBuffer, "r");
	while (fgets(parseBuffer, BUFFER_SIZE, fp)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		token = strrchr(parseBuffer, ' ') + 1;
		entryToken = strtok(parseBuffer, ":");
		if (!_strcmpi(entryToken, "Parent Commit")) {
			if (_strcmpi(token, "NULL")) {
				temp = AllocateCommit();
				strcpy(temp->commitPath, path);
				path = strrchr(path, '\\') + 1;
				sscanf(path, "%d", &(temp->commitID));
				token[strcspn(token, "\n")] = '\0';
				while (heads != NULL && heads->commitPointer!=NULL) {
					commonAncestor = CheckPathway(heads->commitPointer, token);
					if (commonAncestor != NULL) {
						temp->parentCommit = commonAncestor;
						fclose(fp);
						return temp;					
					}
					heads = heads->nextHead;					
				}
				temp->parentCommit = ConstructCommitTree(token, firstHead);
			}
			else {
				temp = AllocateCommit();
				path = strrchr(path, '\\') + 1;
				sscanf(path, "%d", &(temp->commitID));
				temp->parentCommit = NULL;
			}

		}
		else if (!_strcmpi(entryToken, "Branch")) {
			strcpy(temp->branchName, token);
		}
	}

	fclose(fp);
	return temp;
}