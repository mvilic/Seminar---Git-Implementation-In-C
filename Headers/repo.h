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
#				Memory Management				  #
###################################################
*/

int DeallocateRepo(Repo repo);

/*
###################################################
#			Initialisation Routine				  #
###################################################
*/

//Initializes the repository object upon application launch
Repo GitInit();

//Reads references to head commits of all branches in the repository
//Instantiates the complete repository commit tree from read references
int GetHeads(Head heads, char* gitDir);

/*
###################################################
#			Repository Operations				  #
###################################################
*/

Commit Checkout(Repo repo); 
int Branch(Head headCommits, Commit activeCommit);
Commit Merge(Commit toMerge, Head heads);
Commit PushCommit(char* activeDirPath, Commit parentCommit, Head heads);
int History(Head headCommits);
int FilesList(Head headCommits);
int ShowActiveCommit(Commit activeCommit);
#endif

