#include "repo.h"

/*Repo CreateRepo(char* name) {
	Repo temp = NULL;

	temp = (Repo)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	strcpy(temp->repoName, name);
	temp->repoID = rand() % 1999999 + 1;

	temp->gitdir = ".\\.git";

	if (DirectoryExists(temp->gitdir) == RETURN_NOEXIST)
		printf("");

	return temp;
}*/