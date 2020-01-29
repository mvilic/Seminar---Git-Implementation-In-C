#ifndef HEADER_FILE_MANIP_
#define HEADER_FILE_MANIP_

#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include "commit.h"

/*
###################################################
#				General Functions				  #
###################################################
*/

int ClearDirectory(char*);

/*
###################################################
#					Commiting					  #
###################################################
*/

int CreateCommitOnDisk(FolderNode, int, char*); int CommitFiles(FileNode, char*);

/*
###################################################
#					Checking Out				  #
###################################################
*/

int CheckoutCommit(FolderNode, int, char*); 
int CheckoutFiles(FileNode, char*);

#endif