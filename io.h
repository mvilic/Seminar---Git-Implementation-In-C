#ifndef HEADER_IO_
#define HEADER_IO_

#include "common.h"
#include "tree.h"
#include "commit.h"
#include "repo.h"
#include "file_manip.h"

int ConstructTreeFromFiles(TreeNode, char*);
Commit ConstructCommitTree(char*);
Commit FindParent(char*);



#endif
