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

//Clear contents of passed directory
int ClearDirectory(char*);

/*
###################################################
#					Commiting					  #
###################################################
*/

//Create commit file tree in the commits' folder that aren't foreign references
//The passed folder tree holds files that need to be committed, but their path still holds the active directory
int CreateCommitOnDisk(FolderNode, int, char*); 

//Copy files that aren't foreign references
int CommitFiles(FileNode, char*);

/*
###################################################
#					Checking Out				  #
###################################################
*/

//Create folder tree of given commit in the active directory
//Follows very similar logic to the commit creation methods
int CheckoutCommit(FolderNode, int, char*); 

//Copy the passed file list into the passed folder
//Logic identical to file commit method, but disregards foreign status
int CheckoutFiles(FileNode, char*);

#endif