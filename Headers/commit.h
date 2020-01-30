#ifndef HEADER_COMMIT_
#define HEADER_COMMIT_

#include "common.h"
#include "tree.h"

/*
###################################################
#			Structure Definitions				  #
###################################################
*/

struct _commit {
	
	int commitID;
	char commitPath[BUFFER_SIZE];
	char branchName[BUFFER_SIZE];
	char childrenNumber;
	SYSTEMTIME commitDate;
	struct _commit* parentCommit;
	struct _folderNode *fileTree;

}; typedef struct _commit* Commit;

struct _head {
	Commit commitPointer;
	struct _head* nextHead;

}; typedef struct _head* Head;

/*
###################################################
#				Memory Management				  #
###################################################
*/

Commit AllocateCommit(); Head AllocateHead();
int DeallocateCommit(Commit toDeallocate); int DeallocateHead(Head toDeallocate);
int DeallocateBranch(Commit toDeallocate);

/*
###################################################
#				General Functions				  #
###################################################
*/

Commit CommonAncestor(Commit commit1, Commit commit2); Commit CheckPathway(Commit commit, char* path); 
int PrintBranchHistory(Commit currentCommit, Commit headOfBranch);
int ListBranches(Head headCommits); int ListCommitFiles(FolderNode folderTree);
Commit FindBranch(Head headCommits, char* branchName);

/*
###################################################
#				Foreign References				  #
###################################################
*/

int ForeignReferences(Commit); int InsertForeignReference(FolderNode, char*);
int HandleParentForeigns(FolderNode, FolderNode, char*); int PushForeignReferences(FolderNode, char*);
int BranchForeignReferences(Commit branchedCommit, Commit parentCommit);

/*
###################################################
#					Merging						  #
###################################################
*/

int MergePass(FolderNode toMergeTree, FolderNode mergeIntoTree, FolderNode commonAncestorTree, char* mergeOriginCommitPath);

#endif