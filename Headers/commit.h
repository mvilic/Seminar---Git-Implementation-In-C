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

//Recursively iterate through a branch and insert any foreign references found in config files
int ForeignReferences(Commit);

//Inserts a single foreign reference read from the commit config file into the commit's instantiated file tree
int InsertForeignReference(FolderNode, char*);

//Checks for changes made in active directory files to files that are foreign references in the current head of the branch being committed into
int HandleParentForeigns(FolderNode, FolderNode, char*); 

//Push all files marked as foreign references into the commits' config file as references
int PushForeignReferences(FolderNode, char*);

//Carry over parent's foreign references while branching
int BranchForeignReferences(Commit branchedCommit, Commit parentCommit);

/*
###################################################
#					Merging						  #
###################################################
*/

//Take the file tree of the commit being merged into another branch as well as the head commit of the branch being merged into
//For every file in the head of the branch being merged into, check for changes between the commit being merged and the common ancestor of both commits and act accordingly
int MergePass(FolderNode toMergeTree, FolderNode mergeIntoTree, FolderNode commonAncestorTree, char* mergeOriginCommitPath);

#endif