#include "../Headers/commit.h"
#include "../Headers/utility.h"
#include "../Headers/io.h"
#include "../Headers/file_manip.h"

Commit AllocateCommit() {
	Commit temp = (Commit)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	temp->parentCommit = NULL;
	temp->fileTree = NULL;
	temp->commitID = 0;
	temp->childrenNumber = 0;

	GetLocalTime(&(temp->commitDate));
	temp->commitID = temp->commitDate.wYear + temp->commitDate.wMonth + temp->commitDate.wDay + temp->commitDate.wHour + temp->commitDate.wMinute + temp->commitDate.wSecond;
	snprintf(temp->commitPath, BUFFER_SIZE, "sample_repo/.git/.commits/%d", temp->commitID);

	return temp;
}

int DeallocateCommit(Commit toDeallocate) {
	
	if (toDeallocate == NULL)
		return RETURN_OK;

	DeallocateFolderNode(toDeallocate->fileTree);
	free(toDeallocate);
	return RETURN_OK;
}

int DeallocateBranch(Commit commit) {

	if (commit == NULL)
		return RETURN_OK;

	if (commit->childrenNumber > 1)
		return RETURN_OK;
	
	if (commit->parentCommit == NULL) {
		DeallocateFolderNode(commit->fileTree);
		free(commit);
		return RETURN_OK;
	}

	DeallocateFolderNode(commit->fileTree);
	DeallocateBranch(commit->parentCommit);
	free(commit);
	
	return RETURN_OK;
}

int DeallocateHead(Head head) {

	DeallocateCommit(head->commitPointer);
	free(head);

	return RETURN_OK;
}

Head AllocateHead() {
	Head temp = (Head)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	temp->commitPointer = NULL;
	temp->nextHead = NULL;

	return temp;
}

Commit FindCommit(Commit currentCommit, int idToFind) {

	if (currentCommit == NULL)
		return NULL;
	else if (currentCommit->commitID == idToFind)
		return currentCommit;

	return FindCommit(currentCommit->parentCommit, idToFind);
}

Commit CommonAncestor(Commit commit1, Commit commit2) {
	Commit commit2Start = NULL;

	if (commit1 == NULL || commit2 == NULL)
		return NULL;

	commit2Start = commit2;
	while (commit1->parentCommit != NULL) {
		while (commit2->parentCommit != NULL) {
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

int ForeignReferences(Commit currentCommit) {
	FILE* fp = NULL;
	char indexFilePath[BUFFER_SIZE];
	char parseBuffer[BUFFER_SIZE];
	char* pathToken = NULL;
	char* entryToken = NULL;
	char* commitID = NULL;

	while (currentCommit->parentCommit != NULL) {
		sprintf(indexFilePath, "%s/%s", currentCommit->commitPath, ".commit");
		fp = fopen(indexFilePath, "r");
		if (fp == NULL)
			return RETURN_WARNING_FILE_OPEN;

		while (fgets(parseBuffer, BUFFER_SIZE, fp)) {
			parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
			pathToken = strrchr(parseBuffer, ' ') + 1;
			entryToken = strtok(parseBuffer, ":");
			
			if (!_strcmpi(entryToken, "Foreign"))			
				InsertForeignReference(currentCommit->fileTree, pathToken);
		}

		currentCommit = currentCommit->parentCommit;
	}

	fclose(fp);
	return RETURN_OK;
}

int InsertForeignReference(FolderNode commitFolder, char* path) {

	char auxPathBuffer[BUFFER_SIZE];
	char* fileName = NULL, *token = NULL;
	char currentPathBuffer[BUFFER_SIZE]; //drzi path koji se na kraju upise u file node, gradi se postepeno sa svakim tokenom
	char newFolderPath[BUFFER_SIZE];
	FolderNode tempFolder = NULL; FileNode tempFile = NULL;
	FolderNode currentWorkingFolder = NULL;
	FolderNode parentFolder = commitFolder;
	int errnum = 0;
	
	strcpy(auxPathBuffer, path);
	fileName = strrchr(path, '/') + 1;
	token = strtok(path, "/");

	currentWorkingFolder = commitFolder->firstChild;
	sprintf(currentPathBuffer, "%s/%s", "sample_repo/.git/.commits", token); //spremi strani commitID u file path
	while (token = strtok(NULL, "/")) {
		sprintf(currentPathBuffer, "%s/%s", currentPathBuffer, token);
		if (!_strcmpi(fileName, token)) {
			tempFile = CreateFileNode(currentPathBuffer);
			if (tempFile == NULL)
				return RETURN_ERROR_MEM_ALLOC;

			tempFile->foreignFlag = 1;
			AppendFile(parentFolder, tempFile);
			return RETURN_OK;
		}

		//trazi postoji li token folder u strukturi
		while (currentWorkingFolder!=NULL) {
			//pronasa si ga
			if (!_strcmpi((strrchr(currentWorkingFolder->folderPath, '/') + 1), token)) {
				parentFolder = currentWorkingFolder;
				currentWorkingFolder = parentFolder;
				break;
			}
			//ne postoji, napravi ga
			else if (currentWorkingFolder->nextSibling == NULL) {
				sprintf(newFolderPath, "%s/%s", parentFolder->folderPath, token);
				tempFolder = CreateFolderNode(newFolderPath);
				InsertChild(parentFolder, tempFolder);
				parentFolder = tempFolder;
				currentWorkingFolder = parentFolder;
				break;
			}
			currentWorkingFolder = currentWorkingFolder->nextSibling;
		}
	}
		
	return RETURN_OK;
}

int HandleParentForeigns(FolderNode nativeFileTree, FolderNode foreignFileTree, char* commitFilePath) {
	FolderNode currentFolder = NULL; FileNode currentFile = NULL; FileNode foundFile = NULL;
	FILE* commitFile = NULL; 
	char* pathToFile = NULL, * tempHash = NULL, *foreignRef;
	char filePathBuffer[BUFFER_SIZE];
	int errnum = 0;

	currentFolder = foreignFileTree;

	while (currentFolder!=NULL) {
		
		if (currentFolder->firstChild != NULL)
			errnum=HandleParentForeigns(nativeFileTree, currentFolder->firstChild, commitFilePath);
		
		if (errnum != RETURN_OK)
			return errnum;

		commitFile = fopen(commitFilePath, "a");
		if (commitFile == NULL)
			return RETURN_WARNING_FILE_OPEN;

		currentFile = currentFolder->fileList;
		while (currentFile != NULL) {
			if (currentFile->foreignFlag == 1) { //file u parentu koji je stranac.
				pathToFile = strstr(currentFile->filePath, ".commits");
				pathToFile = strchr(pathToFile, '/') + 1;
				pathToFile = strchr(pathToFile, '/') + 1;
				snprintf(filePathBuffer, BUFFER_SIZE, "%s/%s", nativeFileTree->folderPath, pathToFile);
				tempHash = hash(filePathBuffer);

				if (tempHash == NULL) {
					currentFile = currentFile->nextFile;
					continue;
				}
				else if (!_strcmpi(currentFile->fileHash, tempHash)) {
					commitFile = fopen(commitFilePath, "a");
					if (commitFile == NULL)
						return RETURN_WARNING_FILE_OPEN;

					foreignRef = (strstr(currentFile->filePath, ".commits") + 9);
					fprintf(commitFile, "Foreign: %s\n", foreignRef);
					fclose(commitFile);
					free(tempHash);
					foundFile=FindFile(nativeFileTree, filePathBuffer);
					foundFile->foreignFlag = 1;
				}
			}

			currentFile = currentFile->nextFile;
		}

		fclose(commitFile);	
		currentFolder = currentFolder->nextSibling;
	}

	return RETURN_OK;
}

int PushForeignReferences(FolderNode nativeFileTree, char *commitFilePath) {

	FolderNode currentFolder = NULL; FileNode currentFile = NULL;
	FILE* commitFile = NULL; char* temp = NULL, * temp2 = NULL;
	currentFolder = nativeFileTree;
	int errnum = 0;

	while (currentFolder != NULL) {

		if (currentFolder->firstChild != NULL)
			errnum=PushForeignReferences(currentFolder->firstChild, commitFilePath);
		
		if (errnum != RETURN_OK)
			return errnum;

		commitFile = fopen(commitFilePath, "a");
		if (commitFile == NULL)
			return RETURN_WARNING_FILE_OPEN;

		currentFile = currentFolder->fileList;
		while (currentFile != NULL) {
			temp2 = strstr(currentFile->filePath, "active_directory");
			if (currentFile->foreignFlag != 0 && temp2==NULL) {
				temp = strstr(currentFile->filePath, ".commits")+9;
				fprintf(commitFile, "Foreign: %s\n", temp);
			}

			currentFile = currentFile->nextFile;
		}

		fclose(commitFile);
		currentFolder = currentFolder->nextSibling;
	}

	return RETURN_OK;
}

int BranchForeignReferences(Commit branchedCommit, Commit parentCommit) {

	FILE* fp = NULL;
	char indexFilePath[BUFFER_SIZE];
	char parseBuffer[BUFFER_SIZE];
	char* pathToken = NULL;
	char* entryToken = NULL;
	char* commitID = NULL;
	int errnum = 0;

	sprintf(indexFilePath, "%s/%s", parentCommit->commitPath, ".commit");
	fp = fopen(indexFilePath, "r");
	if (fp == NULL)
		return RETURN_WARNING_FILE_OPEN;

	while (fgets(parseBuffer, BUFFER_SIZE, fp)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		pathToken = strrchr(parseBuffer, ' ') + 1;
		entryToken = strtok(parseBuffer, ":");

		if (!_strcmpi(entryToken, "Foreign"))
			errnum=InsertForeignReference(branchedCommit->fileTree, pathToken);

		if (errnum != RETURN_OK)
			return errnum;
	}

	fclose(fp);
	return RETURN_OK;


}

int MergePass(FolderNode toMergeTree, FolderNode mergeIntoTree, FolderNode commonAncestorTree, char* mergeOriginCommitPath) {
	FolderNode currentToMergeFolder = NULL, currentMergeIntoFolder = NULL;
	FileNode currentToMergeFile = NULL, currentMergeIntoFile = NULL, currentCommonAncestorFile = NULL;
	char* pathToFile = NULL;
	char pathBuffer[BUFFER_SIZE], pathBufferAncestor[BUFFER_SIZE], exPathToFile[BUFFER_SIZE];
	int choice = 0, errnum = 0;

	currentToMergeFolder = toMergeTree;
	currentMergeIntoFolder = mergeIntoTree;

	while (currentMergeIntoFolder != NULL) {
		if (currentMergeIntoFolder->firstChild != NULL)
			errnum=MergePass(toMergeTree, currentMergeIntoFolder->firstChild, commonAncestorTree, mergeOriginCommitPath);

		if (errnum != RETURN_OK)
			return errnum;

		currentMergeIntoFile = currentMergeIntoFolder->fileList; //fileovi iz master brancha
		while (currentMergeIntoFile != NULL) {
			pathToFile = strstr(currentMergeIntoFile->filePath, ".commits"); //path to file postaje .commits/2118/..../file.c
			pathToFile = strchr(pathToFile, '/') + 1; //path to file postaje 2118/.../file.c
			strcpy(exPathToFile, pathToFile); //####/.../file.c se sprema za ubacivanje u stablo
			pathToFile = strchr(pathToFile, '/') + 1; //path to file postaje .../file.c
			snprintf(pathBuffer, BUFFER_SIZE, "%s/%s", mergeOriginCommitPath, pathToFile); //path to file postaje sample_repo/.git/.commits/2547/.../file.c

			currentToMergeFile = FindFile(toMergeTree, pathBuffer); //pronadji u ishodistu mergea file iz odredista
			if (currentToMergeFile == NULL) { //ako ga nisi nasa dodaj ga u novi commit
				errnum=InsertForeignReference(toMergeTree, exPathToFile);
				if (errnum != RETURN_OK)
					return errnum;

				currentMergeIntoFile = currentMergeIntoFile->nextFile;
				continue;
			}
			else if (currentToMergeFile->fileHash == currentMergeIntoFile->fileHash) {//ako su im isti hashevi, radi se o istom fileu, ne treba nista raditi
				currentMergeIntoFile = currentMergeIntoFile->nextFile;
				continue;
			}
			else { //ako postoje u odredistu i ishodistu na istom mistu dva istoimena filea sa razlicitim hashevima
				snprintf(pathBufferAncestor, BUFFER_SIZE, "%s/%s", commonAncestorTree->folderPath, pathToFile);
				currentCommonAncestorFile = FindFile(commonAncestorTree, pathBufferAncestor); //pronadji u zajednickom ancestoru file o kojem se radi

				//ne postoji u zajednickom ancestoru ili i jedan i drugi su razliciti od zajednickog izvornog filea. oba slucaja znace konflikt
				if (currentCommonAncestorFile == NULL || (_strcmpi(currentMergeIntoFile->fileHash, currentCommonAncestorFile->fileHash))) {
					printf("\nA conflict has occured. Choose which file to keep:\n1: [%s]\n2: [%s]\nChoice: ", currentMergeIntoFile->filePath, currentToMergeFile->filePath);
					choice = Option(1, 2);
					if (choice == 1) { //korisnik odabrao file iz izvorista mergea
						currentMergeIntoFile = currentMergeIntoFile->nextFile;
						continue;
					}
					else { //korisnik odabrao file iz odredista mergea
						strcpy(currentToMergeFile->filePath, currentMergeIntoFile->filePath);
						strcpy(currentToMergeFile->fileHash, currentMergeIntoFile->fileHash);
					}
				}
			}
			currentMergeIntoFile = currentMergeIntoFile->nextFile;
		}
		currentMergeIntoFolder = currentMergeIntoFolder->nextSibling;
	}

	return RETURN_OK;
}

int PrintBranchHistory(Commit currentCommit, Commit headOfBranch) {

	if (currentCommit->parentCommit == NULL) {
		printf("Commit ID: [%d]\tBranch Name: [%s]", currentCommit->commitID, currentCommit->branchName);
		if (currentCommit != headOfBranch)
			printf("\n\n\t/|\\\n\t |\n\n");
		
		return RETURN_OK;
	}

	PrintBranchHistory(currentCommit->parentCommit, headOfBranch);
	printf("Commit ID: [%d]\tBranch Name: [%s]", currentCommit->commitID, currentCommit->branchName);
	if (currentCommit != headOfBranch)
		printf("\n\n\t/|\\\n\t |\n\n");

	return RETURN_OK;
}

int ListBranches(Head headCommits) {
	char numerator = 0;

	while (headCommits->nextHead != NULL) {
		printf("[%d]: [%s]\n", ++numerator, headCommits->commitPointer->branchName);
		headCommits = headCommits->nextHead;
	}

	return RETURN_OK;
}

int ListCommitFiles(FolderNode folderTree) {
	FolderNode currentFolder = folderTree;
	FileNode currentFile = NULL;

	while (currentFolder != NULL) {
		if (currentFolder->firstChild != NULL)
			ListCommitFiles(currentFolder->firstChild);

		currentFile = currentFolder->fileList;
		while (currentFile != NULL) {
			printf("File Name: [%s]\nFile Path: [%s]\n\n", currentFile->fileName, currentFile->filePath);
			currentFile = currentFile->nextFile;
		}

		currentFolder = currentFolder->nextSibling;
	}

	return RETURN_OK;
}

Commit FindBranch(Head headCommits, char* branchName) {

	while (headCommits->nextHead != NULL) {
		if(_strcmpi(headCommits->commitPointer->branchName, branchName)==0);
			return headCommits->commitPointer;
	
		headCommits = headCommits->nextHead;
	}

	return NULL;
}