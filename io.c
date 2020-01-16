#include "io.h"
#include "commit.h"

Commit ConstructBranch(char* path) { //path je put do direktorija ukljucujuci i njega .git/.commits/####
	Commit temp = NULL; FILE* fp = NULL;
	char pathBuffer[2048] = { 0 };
	char parseBuffer[2048] = { 0 };
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