#include "Headers/common.h"
#include "Headers/file_manip.h"
#include "Headers/commit.h"
#include "Headers/io.h"
#include "Headers/repo.h"
#include "Headers/tree.h"
#include "Headers/utility.h"
#include <errno.h>
/*

#########################################################################
		To Do:
				Izbornik
				Error checkanje i handleanje

#########################################################################

*/
int main() {
	Repo repo = NULL;
	FolderNode stagingArea = NULL;
	FILE* fp = NULL;
	int errnum = 0;
	char choice = 0;
	Commit result = NULL;

	repo = GitInit();
	if (repo == NULL) {
		printf("Error while initializing repository. Exiting.");
		RETURN_ERROR_MEM_ALLOC;
	}

	errnum=GetHeads(repo->heads, repo->gitDir);
	if (errnum != NULL) {
		ErrorReport(errnum);
		return errnum;
	}

	repo->HEAD = FindBranch(repo->heads, "Master");

	do {
		PrintMenu("main");
		choice = Option(0, 6);

		switch (choice) {
		case 0:
			DeallocateRepo(repo);
			return RETURN_OK;
		case 1:
			errnum = History(repo->heads);
			if (errnum != RETURN_OK)
				ErrorReport(errnum);
			break;
		case 2:
			errnum = FilesList(repo->heads);
			if (errnum != RETURN_OK)
				ErrorReport(errnum);
			break;
		case 3:
			result = Checkout(repo);
			if(result!=NULL)
				repo->HEAD = result;
			break;
		case 4:
			result= PushCommit(repo->activeDir, repo->HEAD);
			if (result != NULL)
				repo->HEAD = result;
			break;
		case 5:
			errnum = Branch(repo->heads, repo->HEAD);
			break;
		case 6:
			repo->HEAD = Merge(repo->HEAD, repo->heads);
			break;
		default:	
			printf("Invalid entry. Please try again.\n");
			continue;
		}

	} while (1);

	return RETURN_OK;
}