#ifndef HEADER_COMMIT_
#define HEADER_COMMIT_

#include "common.h"
#include "tree.h"

struct _commit {

	int commitID;
	char commitPath[2048];
	char branchName[2048];
	SYSTEMTIME commitDate;
	struct _commit* parentCommit;
	TreeNode fileTree;

}; typedef struct _commit* Commit;

struct _head {
	Commit commitPointer;
	struct _head* nextHead;

}; typedef struct _head* Head;

Commit CreateCommit(TreeNode, Commit);
Commit AllocateCommit();
int GetHeads(Head, char*); Commit CommonAncestor(Commit, Commit);
int FixPathway(Head);

#endif