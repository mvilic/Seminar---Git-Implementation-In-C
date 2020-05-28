#include "../Headers/commit.h"
#include "../Headers/utility.h"
#include "../Headers/io.h"
#include "../Headers/file_manip.h"

/*
###################################################
#				Memory Management				  #
###################################################
*/

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
	temp->commitID += ((rand() % (300 + 1 - 100)) + 100);
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

/*
###################################################
#				General Functions				  #
###################################################
*/

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

Commit FindBranch(Head headCommits, char* branchName) {

	while (headCommits != NULL) {
		if (_strcmpi(headCommits->commitPointer->branchName, branchName) == 0);
		return headCommits->commitPointer;

		headCommits = headCommits->nextHead;
	}

	return NULL;
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

	while (headCommits != NULL) {
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

/*
###################################################
#				Foreign References				  #
###################################################
*/

//Recursively iterate through a branch and insert any foreign references found in config files
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

//Inserts a single foreign reference read from the commit config file into the commit's instantiated file tree
int InsertForeignReference(FolderNode commitFolder, char* path) {

	char auxPathBuffer[BUFFER_SIZE];
	char* fileName = NULL, *token = NULL;
	//Contains the path that eventually becomes a file node's full file path. Built up gradually with tokens
	char currentPathBuffer[BUFFER_SIZE]; 
	char newFolderPath[BUFFER_SIZE];
	FolderNode tempFolder = NULL; FileNode tempFile = NULL;
	FolderNode currentWorkingFolder = NULL;
	FolderNode parentFolder = commitFolder;
	int errnum = 0;
	
	//Get a copy of the passed path for safe tokenization
	strcpy(auxPathBuffer, path);
	fileName = strrchr(path, '/') + 1;
	token = strtok(path, "/");

	currentWorkingFolder = commitFolder->firstChild;
	//Add the foreign commits' ID into the file path
	sprintf(currentPathBuffer, "%s/%s", "sample_repo/.git/.commits", token);
	//Iterate through the entire passed path, token by token, folder by folder
	while (token = strtok(NULL, "/")) {
		//Append newest part of the file path
		sprintf(currentPathBuffer, "%s/%s", currentPathBuffer, token);
		//If you've reached the end of the path and arrived at the file itself
		if (!_strcmpi(fileName, token)) {
			tempFile = CreateFileNode(currentPathBuffer);
			if (tempFile == NULL)
				return RETURN_ERROR_MEM_ALLOC;

			tempFile->foreignFlag = 1;
			AppendFile(parentFolder, tempFile);
			return RETURN_OK;
		}

		//If not, take the token and try to find the latest folder in the file tree.
		while (currentWorkingFolder!=NULL) {
			//If it exists, set pointers and continue onwards
			if (!_strcmpi((strrchr(currentWorkingFolder->folderPath, '/') + 1), token)) {
				parentFolder = currentWorkingFolder;
				currentWorkingFolder = parentFolder;
				break;
			}
			//If it doesn't exist, create it and move onwards into the next level of the file tree
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

//Checks for changes made in active directory files to files that are foreign references in the current head of the branch being committed into
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
			if (currentFile->foreignFlag == 1) { //Foreign reference in parent commit
				pathToFile = strstr(currentFile->filePath, ".commits");
				pathToFile = strchr(pathToFile, '/') + 1;
				pathToFile = strchr(pathToFile, '/') + 1; //Extract file path relative to commit root folder
				snprintf(filePathBuffer, BUFFER_SIZE, "%s/%s", nativeFileTree->folderPath, pathToFile); //Construct path to file in respect to the commit being pushed
				tempHash = hash(filePathBuffer);

				//File not found, meaning it was removed from the project in the current version
				if (tempHash == NULL) {
					currentFile = currentFile->nextFile;
					continue;
				}
				//File found, compare hashes to check for changes. Act accordingly
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

//Push all files marked as foreign references into the commits' config file as references
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

//Carry over parent's foreign references while branching
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

/*
###################################################
#					Merging						  #
###################################################
*/

//Take the file tree of the commit being merged into another branch as well as the head commit of the branch being merged into
//For every file in the head of the branch being merged into, check for changes between the commit being merged and the common ancestor of both commits and act accordingly
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
			errnum = MergePass(toMergeTree, currentMergeIntoFolder->firstChild, commonAncestorTree, mergeOriginCommitPath);

		if (errnum != RETURN_OK)
			return errnum;

		currentMergeIntoFile = currentMergeIntoFolder->fileList; //Files from the branch recieving the merge
		while (currentMergeIntoFile != NULL) {
			pathToFile = strstr(currentMergeIntoFile->filePath, ".commits"); //Path to file becomes .commits/####/..../somefile
			pathToFile = strchr(pathToFile, '/') + 1; //Path to file becomes ####/.../somefile
			strcpy(exPathToFile, pathToFile); //####/.../somefile recorded for eventual insertion into the tree
			pathToFile = strchr(pathToFile, '/') + 1; //Path to file becomes relative path from root commit folder .../file.c
			snprintf(pathBuffer, BUFFER_SIZE, "%s/%s", mergeOriginCommitPath, pathToFile); //Join the commit folder path of the commit being merged and the relative path of the file from the recieving branch

			currentToMergeFile = FindFile(toMergeTree, pathBuffer); //Try to find the file from the merge destination branch in the merge origin commit
			if (currentToMergeFile == NULL) { //If not found, it's a new file, add it into the resulting merged commit
				errnum = InsertForeignReference(toMergeTree, exPathToFile);
				if (errnum != RETURN_OK)
					return errnum;

				currentMergeIntoFile = currentMergeIntoFile->nextFile;
				continue;
			}
			else if (currentToMergeFile->fileHash == currentMergeIntoFile->fileHash) {//If they bear the same hash they are the same file, so no action necessary
				currentMergeIntoFile = currentMergeIntoFile->nextFile;
				continue;
			}
			else { //Same file exists in both commits with differing hashes
				snprintf(pathBufferAncestor, BUFFER_SIZE, "%s/%s", commonAncestorTree->folderPath, pathToFile);
				currentCommonAncestorFile = FindFile(commonAncestorTree, pathBufferAncestor); //Find the file in question in their common ancestor

				//File not found in common ancestor or they are both different from the original file. Both cases mean a conflict
				if (currentCommonAncestorFile == NULL || (_strcmpi(currentMergeIntoFile->fileHash, currentCommonAncestorFile->fileHash))) {
					printf("\nA conflict has occured. Choose which file to keep:\n1: [%s]\n2: [%s]\nChoice: ", currentMergeIntoFile->filePath, currentToMergeFile->filePath);
					choice = Option(1, 2);
					if (choice == 1) { //User chose file from origin of merge
						currentMergeIntoFile = currentMergeIntoFile->nextFile;
						continue;
					}
					else { //User chose file from destination of merge
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