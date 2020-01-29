#ifndef HEADER_REPO_
#define HEADER_REPO_

#include "common.h"
#include "commit.h"
#include "file_manip.h"

/*
###################################################
#			Structure Definitions				  #
###################################################
*/

typedef struct _repo {

	char gitDir[BUFFER_SIZE];
	char activeDir[BUFFER_SIZE];
	Head heads;
	Commit HEAD;

}repo; typedef repo* Repo;

/*
###################################################
#			Repository Operations				  #
###################################################
*/

Repo GitInit();
Commit Checkout(Head headCommits, FolderNode stagingArea); 
int Branch(Head headCommits, Commit activeCommit);
Commit Merge(Commit toMerge, Head heads);
int GetHeads(Head heads, char* gitDir);

#endif

