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
				Commit history od brancha
				Izbornik
				Error checkanje i handleanje
				Razna dealokacija

#########################################################################

*/
int main() {
	Repo repo = NULL;
	FolderNode stagingArea = NULL;
	FILE* fp = NULL;
	int errnum = 0;
	char choice = 0;

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

	ListCommitFiles(repo->heads->commitPointer->fileTree);

	do {
		PrintMenu("main");
		choice = Option(0, 9);

		switch (choice) {
		
		
		
		}

	} while (1);
	
	//CheckoutCommit(heads->commitPointer->fileTree, strlen(heads->commitPointer->commitPath), "sample_repo/active_directory");
	/*FolderNode StagingArea = CreateFolderNode("sample_repo/active_directory");
	StageForCommit(StagingArea, StagingArea->folderPath, strlen(StagingArea->folderPath), heads->commitPointer->fileTree);
	PushCommit(StagingArea, heads->commitPointer);*/

	return RETURN_OK;
	}


/*
int main(int argc, char** args) {

	char* command = NULL;

	if (argc < 2) {
		printf("Koristenje: mygit <command>\n");
	}
	
	if (strcmpi(args[1], "add"))
		Add(args[2]);
	else if (strcmpi(args[1], "commit"))
		Commit(args[2]);
	else if (strcmpi(args[1], "merge"))
		Merge(args[2]);
	else if (strcmpi(args[1], "init"))
		Init();
	else if (strcmpi(args[1], "status"))
		Status();
	else
		printf("Nepostojeca komanda.\n");


	return 0;
}*/