#ifndef HEADER_REPO_
#define HEADER_REPO_

#include "common.h"
#include "commit.h"

typedef struct _repo {

	int repoID;
	char* repoName;
	char* creatorName;
	char* gitdir;
	Head commitTree;
	Commit workTree;

}Repo;

Repo CreateRepo(char*);

Checkout(char* mode, char* choice); 
//prima mode:branch/commit i choice string, potrazi choic epreko commitID u aktivnom branchu ili pretrazi u heads po branchName




#endif

