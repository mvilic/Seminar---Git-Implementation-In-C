#include "../Headers/repo.h"
#include "../Headers/utility.h"
#include "../Headers/io.h"

int DeallocateRepo(Repo repo) {
	Head currentHead = repo->heads;
	Head temp = NULL;

	while (currentHead != NULL) {
	
		temp = currentHead;
		currentHead = currentHead->nextHead;
		DeallocateBranch(temp->commitPointer);
		free(temp);
	}

	free(repo);
	return RETURN_OK;
}

Repo GitInit() {
	Repo temp = NULL;

	temp = (Repo)malloc(sizeof(*temp));
	if (temp == NULL)
		return NULL;

	strcpy(temp->gitDir, "sample_repo/.git");
	strcpy(temp->activeDir, "sample_repo/active_directory");
	
	temp->heads = AllocateHead();
	if (temp->heads == NULL) {
		free(temp);
		return NULL;
	}

	temp->HEAD = NULL;
	return temp;
}

int GetHeads(Head heads, char* gitDir) {
	FILE* headsFile = NULL; Head headsStart = heads, temp = NULL;
	char parseBuffer[BUFFER_SIZE];
	char indexPath[BUFFER_SIZE];

	sprintf(indexPath, "%s/%s", gitDir, ".heads");
	headsFile = fopen(indexPath, "r");
	if (headsFile == NULL)
		return RETURN_WARNING_FILE_OPEN;

	while (fgets(parseBuffer, BUFFER_SIZE, headsFile)) {
		parseBuffer[strcspn(parseBuffer, "\n")] = '\0';
		heads->commitPointer = ConstructCommitTree(parseBuffer, headsStart);
		heads->nextHead = AllocateHead();
		ForeignReferences(heads->commitPointer);
		heads = heads->nextHead;
	}

	heads = headsStart;
	while (heads->nextHead->commitPointer != NULL)
		heads = heads->nextHead;

	temp = heads->nextHead;
	heads->nextHead = NULL;
	free(temp);
	fclose(headsFile);
	return RETURN_OK;
}

Commit Checkout(Repo repo) {
	Commit commitToCheckout = NULL;
	FolderNode stagingArea = NULL;
	int choice = 0, errnum = 0;

	printf("Checking out will result in loss of any uncommitted changes. Proceed? [Y/N]: ");
	choice = Choice();
	if (choice == 'n')
		return NULL;

	commitToCheckout = CheckoutInputParse(repo->heads);
	if (commitToCheckout == NULL) {
		ErrorReport(RETURN_USER_ABORT);
		return NULL;
	}
	
	
	ClearDirectory(repo->activeDir);
	errnum=CheckoutCommit(commitToCheckout->fileTree, strlen(commitToCheckout->commitPath), repo->activeDir);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		return errnum;
	}

	return commitToCheckout;
}

int Branch(Head headCommits, Commit activeCommit){
	char newBranchName[BUFFER_SIZE], commitFilePath[BUFFER_SIZE];
	int replacePosition = 0, errnum = 0;
	Commit temp = NULL;
	FILE* headsFile = NULL, * commitFile = NULL;

	//do {
		printf("Enter new branch name. Enter 0 to exit\nInput: ");
		scanf("%s", newBranchName);

		if (!_strcmpi(newBranchName, "0"))
			return RETURN_USER_ABORT;
		
		/*if (FindBranch(headCommits, newBranchName) != NULL) {
			printf("Branch with that name already exists.\n");
			continue;
		}
		else
			break;
	
	} while (1);*/

	temp = AllocateCommit();
	if (temp == NULL)
		return RETURN_ERROR_MEM_ALLOC;

	strcpy(temp->branchName, newBranchName);
	temp->parentCommit = activeCommit;
	temp->fileTree = CreateFolderNode(temp->commitPath);
	if (temp->fileTree == NULL) {
		free(temp);
		return RETURN_ERROR_MEM_ALLOC;
	}
	
	_mkdir(temp->commitPath);
	snprintf(commitFilePath, BUFFER_SIZE, "%s/%s", temp->commitPath, ".commit");
	commitFile = fopen(commitFilePath, "w");
	if (commitFile == NULL) {
		DeallocateCommit(temp);
		return RETURN_WARNING_FILE_OPEN;
	}

	fprintf(commitFile, "Parent Commit: %s\nBranch: %s\n", activeCommit->fileTree->folderPath, temp->branchName);
	fclose(commitFile);

	errnum=ConstructFileTree(temp->fileTree, activeCommit->fileTree->folderPath);
	if (errnum != RETURN_OK) {
		DeallocateCommit(temp);
		return errnum;
	}
	
	errnum=StageForBranch(temp->fileTree);
	if (errnum != RETURN_OK) {
		DeallocateCommit(temp);
		return errnum;
	}
		
	errnum=BranchForeignReferences(temp, activeCommit);
	if (errnum != RETURN_OK) {
		DeallocateCommit(temp);
		return errnum;
	}

	replacePosition = strlen(activeCommit->commitPath);
	errnum=CreateCommitOnDisk(temp->fileTree, replacePosition, temp->commitPath);
	if (errnum != RETURN_OK) {
		DeallocateCommit(temp);
		return errnum;
	}

	errnum=PushForeignReferences(temp->fileTree, commitFilePath);
	if (errnum != RETURN_OK) {
		DeallocateCommit(temp);
		return errnum;
	}

	while (headCommits->nextHead != NULL)
		headCommits = headCommits->nextHead;

	headCommits->nextHead = AllocateHead();
	if(headCommits->nextHead==NULL) {
		DeallocateCommit(temp);
		return RETURN_ERROR_MEM_ALLOC;
	}

	headCommits->nextHead->commitPointer = temp;

	AppendHeadToIndex(temp);
	activeCommit->childrenNumber++;

	return RETURN_OK;
}

Commit PushCommit(char* activeDirPath, Commit parentCommit) { //passedFileTree ce imati path sample_repo/active_directory
	char buffer[BUFFER_SIZE] = { 0 };
	int replacePosition = 0, errnum = 0;
	FILE* commitFile = NULL; FILE* tempFile = NULL;
	FolderNode stagingArea = NULL;

	stagingArea = CreateFolderNode(activeDirPath);
	if (stagingArea == NULL) {
		ErrorReport(RETURN_ERROR_MEM_ALLOC);
		return NULL;
	}

	errnum = StageForCommit(stagingArea, stagingArea->folderPath, strlen(activeDirPath), parentCommit->fileTree);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		DeallocateFolderNode(stagingArea);
		return NULL;
	}

	Commit temp = AllocateCommit();
	if(temp==NULL) {
		ErrorReport(RETURN_ERROR_MEM_ALLOC);
		DeallocateFolderNode(stagingArea);
		return NULL;
	}

	temp->fileTree = stagingArea;
	temp->parentCommit = parentCommit;
	strcpy(temp->branchName, parentCommit->branchName);
	_mkdir(temp->commitPath);
	snprintf(buffer, BUFFER_SIZE, "%s/%s", temp->commitPath, ".commit");
	
	commitFile = fopen(buffer, "w");
	if (commitFile == NULL) {
		ErrorReport(RETURN_WARNING_FILE_OPEN);
		DeallocateCommit(temp);
		return NULL;
	}

	fprintf(commitFile, "Parent Commit: %s\nBranch: %s\n", parentCommit->fileTree->folderPath, parentCommit->branchName);
	fclose(commitFile);

	errnum=HandleParentForeigns(temp->fileTree, parentCommit->fileTree, buffer);
	if (errnum != RETURN_OK) {
		ErrorReport(RETURN_WARNING_FILE_OPEN);
		DeallocateCommit(temp);
		return NULL;
	}

	replacePosition = strlen(stagingArea->folderPath);
	errnum=CreateCommitOnDisk(temp->fileTree, replacePosition, temp->commitPath);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		DeallocateCommit(temp);
		return NULL;
	}
	
	errnum=PushForeignReferences(temp->fileTree, buffer);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		DeallocateCommit(temp);
		return NULL;
	}

	errnum = ReplaceHeadInIndex(temp, parentCommit);
	parentCommit->childrenNumber++;
	return temp;
}

Commit Merge(Commit toMerge, Head heads) {
	Commit mergeInto = NULL, newCommit = NULL, commonAncestor = NULL;
	Head currentHead = heads; FILE* newCommitFile = NULL;
	char newCommitFilePath[BUFFER_SIZE];
	int errnum = 0;

	mergeInto = MergeInputParse(heads);
	if (mergeInto == NULL) {
		ErrorReport(RETURN_USER_ABORT);
		return NULL;
	}

	commonAncestor = CommonAncestor(toMerge, mergeInto);
	if (commonAncestor == NULL) {
		ErrorReport(RETURN_ERROR_COMMON_ANCESTOR);
		return NULL;
	}

	newCommit = AllocateCommit();
	if (newCommit == NULL) {
		ErrorReport(RETURN_ERROR_MEM_ALLOC);
		return NULL;
	}

	strcpy(newCommit->branchName, mergeInto->branchName);
	newCommit->parentCommit = mergeInto;
	newCommit->parentCommit->childrenNumber++;
	_mkdir(newCommit->commitPath);

	newCommit->fileTree = CreateFolderNode(newCommit->commitPath);
	if (newCommit->fileTree == NULL) {
		ErrorReport(RETURN_ERROR_MEM_ALLOC);
		DeallocateCommit(newCommit);
		return NULL;
	}

	snprintf(newCommitFilePath, BUFFER_SIZE, "%s/%s", newCommit->commitPath, ".commit");
	newCommitFile = fopen(newCommitFilePath, "w");
	if (newCommitFile == NULL) {
		ErrorReport(RETURN_WARNING_FILE_OPEN);
		DeallocateCommit(newCommit);
		return NULL;
	}

	fprintf(newCommitFile, "Parent Commit: %s\nBranch: %s\n", mergeInto->fileTree->folderPath, newCommit->branchName);
	fclose(newCommitFile);

	errnum=ConstructFileTree(newCommit->fileTree, toMerge->fileTree->folderPath);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		DeallocateCommit(newCommit);
		return NULL;
	}

	errnum=MergePass(newCommit->fileTree, mergeInto->fileTree, commonAncestor->fileTree, toMerge->commitPath);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		DeallocateCommit(newCommit);
		return NULL;
	}

	StageForBranch(newCommit->fileTree);
	errnum=CreateCommitOnDisk(newCommit->fileTree, strlen(newCommit->commitPath), newCommit->commitPath);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		DeallocateCommit(newCommit);
		return NULL;
	}

	errnum=PushForeignReferences(newCommit->fileTree, newCommitFilePath);
	if (errnum != RETURN_OK) {
		ErrorReport(errnum);
		DeallocateCommit(newCommit);
		return NULL;
	}

	//sredi izmjenu heada
	errnum=ReplaceHeadInIndex(newCommit, mergeInto);
	if(errnum!=RETURN_OK) {
		ErrorReport(errnum);
		DeallocateCommit(newCommit);
		return NULL;
	}

	return newCommit;
}

int History(Head headCommits) {
	Commit chosenCommit = NULL;
	int errnum = 0;

	printf("\n\nPrint history of which branch?\n");
	ListBranches(headCommits);

	chosenCommit = CheckoutInputParse(headCommits);
	
	if (chosenCommit == NULL)
		return RETURN_USER_ABORT;

	errnum=PrintBranchHistory(chosenCommit, chosenCommit);
	return RETURN_OK; 
}

int FilesList(Head headCommits) {
	Commit chosenCommit = NULL;
	
	chosenCommit = CheckoutInputParse(headCommits);
	if (chosenCommit == NULL)
		return RETURN_USER_ABORT;

	ListCommitFiles(chosenCommit->fileTree);
	return RETURN_OK;
}
