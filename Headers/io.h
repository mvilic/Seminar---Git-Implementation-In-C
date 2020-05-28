#ifndef HEADER_IO_
#define HEADER_IO_

#include "common.h"
#include "tree.h"
#include "commit.h"
#include "repo.h"
#include "file_manip.h"

/*
###################################################
#				Repository Loading				  #
###################################################
*/

//Constructs a single branch from a head commit entry in the index file
Commit ConstructBranch(char*);

//Construct complete tree for given head commit
Commit ConstructCommitTree(char*, Head);

//Instantiates and constructs the passed commits' file tree using the passed folder node as the root folder
int ConstructFileTree(FolderNode, char*);


#endif