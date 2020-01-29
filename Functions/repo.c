#include "../Headers/repo.h"
#include "../Headers/utility.h"
#include "../Headers/io.h"

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

Commit Checkout(Head headCommits, FolderNode stagingArea) {
	Commit commitToCheckout = NULL;
	Head currentHead = headCommits;
	int choice=0, stagingStatus = 0;
	

	CheckFilestate(stagingArea, &stagingStatus);

	if (stagingStatus != 0) {
		printf("Staged files waiting for commit. Checking out will result in loss of all changes. Proceed? [Y/N]\n");
	}
	else
		printf("Checking out will result in loss of any uncommitted changes. Proceed? [Y/N]");

	choice = Choice();

	if (choice == 'n')
		return NULL;

	commitToCheckout = CheckoutInputParse(headCommits);
	
	if (commitToCheckout == NULL)
		return NULL;
	
	ClearDirectory(stagingArea->folderPath);
	CheckoutCommit(commitToCheckout->fileTree, strlen(commitToCheckout->fileTree->folderPath), stagingArea->folderPath);

	return commitToCheckout;
}

int Branch(Head headCommits, Commit activeCommit){
	char newBranchName[BUFFER_SIZE], commitFilePath[BUFFER_SIZE];
	int replacePosition = 0;
	Head currentHead = NULL;
	Commit temp = NULL;
	FILE* headsFile = NULL, * commitFile = NULL;

	printf("Enter new branch name. Enter 0 to exit\nInput: ");
	scanf("%s", newBranchName);

	if (!_strcmpi(newBranchName, "0"))
		return RETURN_USER_ABORT;

	temp = AllocateCommit();
	
	strcpy(temp->branchName, newBranchName);
	temp->parentCommit = activeCommit;
	temp->fileTree = CreateFolderNode(temp->commitPath);
	_mkdir(temp->commitPath);
	snprintf(commitFilePath, BUFFER_SIZE, "%s/%s", temp->commitPath, ".commit");
	commitFile = fopen(commitFilePath, "w");
	fprintf(commitFile, "Parent Commit: %s\nBranch: %s\n", activeCommit->fileTree->folderPath, activeCommit->branchName);
	fclose(commitFile);

	ConstructFileTree(temp->fileTree, activeCommit->fileTree->folderPath);
	StageForBranch(temp->fileTree);
	BranchForeignReferences(temp, activeCommit);

	replacePosition = strlen(activeCommit->commitPath);
	CreateCommitOnDisk(temp->fileTree, replacePosition, temp->commitPath);
	PushForeignReferences(temp->fileTree, commitFilePath);

	while (headCommits->nextHead != NULL)
		headCommits = headCommits->nextHead;

	headCommits->nextHead->commitPointer = temp;

	AppendHeadToIndex(temp);
	activeCommit->childrenNumber++;

	return RETURN_OK;
}

Commit Merge(Commit toMerge, Head heads) {
	Commit mergeInto = NULL, newCommit = NULL, commonAncestor = NULL;
	Head currentHead = heads; FILE* newCommitFile = NULL;
	char newCommitFilePath[BUFFER_SIZE];

	mergeInto = MergeInputParse(heads);
	if (mergeInto == NULL)
		return NULL;

	commonAncestor = CommonAncestor(toMerge, mergeInto);
	if (commonAncestor == NULL) {
		return NULL;
	}

	newCommit = AllocateCommit();
	if (newCommit == NULL)
		return NULL;

	strcpy(newCommit->branchName, mergeInto->branchName);
	newCommit->parentCommit = mergeInto;
	newCommit->parentCommit->childrenNumber++;
	_mkdir(newCommit->commitPath);

	newCommit->fileTree = CreateFolderNode(newCommit->commitPath);
	if (newCommit->fileTree == NULL) {
		DeallocateCommit(newCommit);
		return NULL;
	}

	snprintf(newCommitFilePath, BUFFER_SIZE, "%s/%s", newCommit->commitPath, ".commit");
	newCommitFile = fopen(newCommitFilePath, "w");
	fprintf(newCommitFile, "Parent Commit: %s\nBranch: %s\n", mergeInto->fileTree->folderPath, newCommit->branchName);
	fclose(newCommitFile);

	ConstructFileTree(newCommit->fileTree, toMerge->fileTree->folderPath);
	MergePass(newCommit->fileTree, mergeInto->fileTree, commonAncestor->fileTree, toMerge->commitPath);

	StageForBranch(newCommit->fileTree);
	CreateCommitOnDisk(newCommit->fileTree, strlen(newCommit->commitPath), newCommit->commitPath);
	PushForeignReferences(newCommit->fileTree, newCommitFilePath);

	//sredi izmjenu heada

	return newCommit;
}

int History(Head headCommits) {
	Commit chosenCommit = NULL;
	int errnum = 0;

	printf("Print history of which branch?");
	ListBranches(headCommits);

	chosenCommit = CheckoutInputParse(headCommits);
	
	if (chosenCommit == NULL)
		return RETURN_USER_ABORT;

	errnum=PrintBranchHistory(chosenCommit, chosenCommit);
	return RETURN_OK; 
}

