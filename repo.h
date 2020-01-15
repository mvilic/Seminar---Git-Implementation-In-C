#ifndef HEADER_REPO_
#define HEADER_REPO_

#include "common.h"
#include "commit.h"

struct _repo {

	int repoID;
	char* repoName;
	char* creatorName;
	char* gitdir;
	Commit commitTree;
	Commit workTree;

}; typedef struct _repo* Repo;

Repo CreateRepo(char*);

#endif

