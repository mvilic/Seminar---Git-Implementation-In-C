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

	sprintf(buffer, "sample_repo/.git/.commits/%d", temp->commitID);

	strcpy(temp->commitPath, buffer);

	return temp;
}

int GetHeads(Head heads, char* gitDir) {
	FILE* headsFile = NULL; Head headsStart = heads;
	char parseBuffer[2048];
	char indexPath[2048];

	sprintf(indexPath, "%s/%s", gitDir, ".heads");
	headsFile = fopen(indexPath, "r");
	while (fgets(parseBuffer, BUFFER_SIZE, headsFile)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		heads->commitPointer = ConstructCommitTree(parseBuffer, headsStart);
		heads->nextHead = AllocateHead();
		ForeignReferences(heads->commitPointer);
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

int ForeignReferences(Commit commitHead) { //ne stvaraj novi nego procitaj i iteriraj po branchu dok ne dodjes, pa izjednaci pokazivace
	FILE* fp = NULL;
	char indexFilePath[BUFFER_SIZE];
	char parseBuffer[BUFFER_SIZE];
	char* token = NULL;
	char* entryToken = NULL;
	char* commitID = NULL;

	while (commitHead->parentCommit != NULL) {
		sprintf(indexFilePath, "%s/%s", commitHead->commitPath, ".commit");
		fp = fopen(indexFilePath, "r");
		if (fp == NULL)
			return RETURN_WARNING_FILE_OPEN;

		while (fgets(parseBuffer, BUFFER_SIZE, fp)) {
			parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
			entryToken = strtok(parseBuffer, ":");									//parse buffer postaje commitID\....\filename
			if (!_strcmpi(entryToken, "ForeignFile")) {
				token = strtok(NULL, "/");											//parse buffer postaje ....\filename

			}


		}

		
		commitHead = commitHead->parentCommit;
	}
	return RETURN_OK;
}

//current Folder je root folder stabla u commitu. path je putanja do filea bez commitID foldera
int InsertForeignReference(FolderNode parentFolder, char* path, char* foreignCommitID, char* nativeCommitID) {
	char* folderToken = NULL;
	char* fileToken = NULL; char* targetFileToken = NULL;
	char* commitFolderToken = NULL;
	char targetPathBuffer[BUFFER_SIZE];
	FileNode currentFile = parentFolder->fileList;
	FolderNode firstOfLevel = NULL;
	FolderNode currentFolder = NULL;
	FolderNode temp = NULL;
	FILE* fp = NULL;


	fileToken = strrchr(path, '/'); //izvuci ime filea
	//budaletino imas gotov kod u io. samo construct file tree uz extra provjere
	if (fileToken == NULL) {
		while (currentFile->nextFile != NULL)
			currentFile = currentFile->nextFile;

		strcpy(targetPathBuffer, parentFolder->folderPath);
		targetFileToken = strtok(targetPathBuffer, nativeCommitID);
		sprintf(targetPathBuffer, "%s%s/%s", targetFileToken, foreignCommitID, path);
		fp = fopen(targetPathBuffer, "r");
		fileToken = hash(targetPathBuffer);
		currentFile->nextFile = CreateFileNode(targetPathBuffer);
		return RETURN_OK;
	}

	fileToken++;
	strcpy(targetPathBuffer, path);
	currentFolder = parentFolder->firstChild;
	firstOfLevel = currentFolder;
	folderToken = strtok(targetPathBuffer, "/"); //prvi folder u pathu
	
	while (folderToken != NULL) {
		while (currentFolder != NULL) {
			if (strstr(currentFolder->folderPath, folderToken)) {
				InsertForeignReference(currentFolder, DelimiterSlice(path, '/'), foreignCommitID, nativeCommitID);
				break;
			}
			else {
				currentFolder = currentFolder->nextSibling;
			}
			
			currentFolder->nextSibling;
		
		}
		

		folderToken = strtok(NULL, "/");
	}
	

}

int InsertForeignReference2(FolderNode commitFolder, char* path) { //prvobitno se kao parent folder salje sample_repo/.git/.commits/#### -- dalje bi se rekurzivno triba slati first child

	/*char* pathTokens[20];
	char auxPathBuffer[BUFFER_SIZE];
	int i = 0, j = 0, fileNamePosition = 0;

	strcpy(auxPathBuffer, path);

	pathTokens[i++] = strtok(path, "/");
	while ((pathTokens[i] = strtok(NULL, "/")) != NULL) //iman niz foldera u pathu, od kojih je zadnji ime datoteke
		i++;

	fileNamePosition = i-1;*/

	char auxPathBuffer[BUFFER_SIZE];
	char* fileName = NULL, token = NULL;
	char currentPathBuffer[BUFFER_SIZE];
	FolderNode tempFolder = NULL; FileNode tempFile = NULL;
	FolderNode currentWorkingFolder = NULL; FileNode currentFile = NULL;

	//path="2365/headers/frgn.txt"
	//parentFolder->folderPath=sample_repo/.git/.commits/####
	currentWorkingFolder = commitFolder->firstChild;
	strcpy(auxPathBuffer, path);
	fileName = strrchr(path, '/') + 1;
	token = strtok(path, '/');
	sprintf(currentPathBuffer, "%s/%s", commitFolder->folderPath, token);
	while (token = strtok(NULL, '/')) {
		if (!_strcmpi(token, fileName)) {//zamini hardkodirano sample_repo/.git sa gitdir
			sprintf(currentPathBuffer, "sample_repo/.git/.commits/%s", auxPathBuffer);
			tempFile = CreateFileNode(currentPathBuffer);

			AppendFile(commitFolder, tempFile);
			break;
		}
		else{
			sprintf(currentPathBuffer, "%s/%s", currentPathBuffer, token);
			while (currentWorkingFolder->nextSibling != NULL) {
				if (!_strcmpi(currentWorkingFolder->folderPath, currentPathBuffer)) {
					currentWorkingFolder = currentWorkingFolder->firstChild;
				
				
				}
				
				currentWorkingFolder = currentWorkingFolder->nextSibling;
			}
			InsertChild(commitFolder, tempFolder);
		
		}
		
	
	
	
	
	
	}




	return RETURN_OK;
}