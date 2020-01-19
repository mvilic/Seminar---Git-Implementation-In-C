#include "common.h"
#include "file_manip.h"
#include <lmcons.h>
#include "commit.h"
#include "tree.h"
#include "repo.h"

int main() {	
	/*Head heads = AllocateHead(); Commit commonAncestor = NULL;
	char gitDir[2048] = "sample_repo\\.git";
	GetHeads(heads, gitDir);*/

	return 0;
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