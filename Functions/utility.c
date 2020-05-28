#include "../Headers/utility.h"

char* hash(char* filePath)
{
	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	BYTE rgbFile[6000];
	DWORD cbRead = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";
	wchar_t filename[BUFFER_SIZE];
	int j = 0;

	char *result=(char*)malloc(MD5LEN * 2 + 1);

	if (result == NULL)
		return NULL;

	mbstowcs(filename, filePath, BUFFER_SIZE);

	hFile = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		return NULL;
	}

	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		return NULL;
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		CryptReleaseContext(hProv, 0);
		return NULL;
	}

	while (bResult = ReadFile(hFile, rgbFile, BUFFER_SIZE,
		&cbRead, NULL))
	{
		if (0 == cbRead)
		{
			break;
		}

		if (!CryptHashData(hHash, rgbFile, cbRead, 0))
		{
			dwStatus = GetLastError();
			printf("CryptHashData failed: %d\n", dwStatus);
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return NULL;
		}
	}

	if (!bResult)
	{
		dwStatus = GetLastError();
		printf("ReadFile failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return NULL;
	}

	cbHash = MD5LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		for (DWORD i = 0; i < cbHash; i++)
		{
			result[j++] = rgbDigits[rgbHash[i] >> 4];
			result[j++] = rgbDigits[rgbHash[i] & 0xf];
		}
		result[j] = '\0';
	}
	else
	{
		dwStatus = GetLastError();
		printf("CryptGetHashParam failed: %d\n", dwStatus);
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);


	return result;
}

int DirectoryExists(char* path) {
	struct stat info;

	if (stat(path, &info) != 0)
		return RETURN_NOEXIST;
	else if (info.st_mode & S_IFDIR)
		return RETURN_EXIST;
	else
		return RETURN_NOTDIR;
}

int ListDirectoryContents(const wchar_t* sDir)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	wchar_t sPath[BUFFER_SIZE];

	//file mask. *.* = everything 
	wsprintf(sPath, L"%s/*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return RETURN_NOEXIST;
	}

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
		{ 
			wsprintf(sPath, L"%s/%s", sDir, fdFile.cFileName);

			//File or Folder? 
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wprintf(L"Directory: %s\n", sPath);
				ListDirectoryContents(sPath);
			}
			else {
				wprintf(L"File: %s\n", sPath);
			}
		}
	} while (FindNextFile(hFind, &fdFile));

	FindClose(hFind);

	return RETURN_OK;
}

char* DelimiterSlice(char* string, char delimiter) {

	char* result = NULL;
	result = string;

	while (result != NULL && result[0] != delimiter)
		result++;

	while (result[0] == delimiter)
		result++;

	return result;

}

int Choice() {
	int result = 0;
	char lineBuffer[BUFFER_SIZE];

	do {
		fgets(lineBuffer, BUFFER_SIZE, stdin);
		if (lineBuffer[0] == '\n')
			continue;
		
		lineBuffer[strlen(lineBuffer) - 1] = '\0';
		if (sscanf(lineBuffer, "%c", &result) != 1 || (result!='y' && result!='n')) {
			printf("Invalid choice.\nTry again: ");
			continue;
		}
		else
			break;
	} while (1);

	return tolower(result);
}

int Option(int lwr_limit, int uppr_limit) {
	int result = 0; char buffer[BUFFER_SIZE];

	do {
		fgets(buffer, BUFFER_SIZE, stdin);
		if (buffer[0] == '\n')
			continue;

		if (sscanf(buffer, "%d", &result) != 1 || result > uppr_limit || result < lwr_limit) {
			printf("Invalid choice. Try again.\nInput: ");
			continue;
		}
		else
			break;
	} while (1);

	return result;
}

Commit CheckoutInputParse(Head heads) {
	char lineBuffer[BUFFER_SIZE];
	Head currentHead = heads;
	int choice = 0;
	Commit selectedCommit = NULL;

	do {
		printf("Enter commit ID or branch name.\nTo abort, enter 0.\nInput: ");
		fgets(lineBuffer, BUFFER_SIZE, stdin);

		if (lineBuffer[0] == '\n')
			continue;
		else if (lineBuffer[0] == 0)
			return NULL;

		lineBuffer[strlen(lineBuffer)-1] = '\0';

		//provjeri jeli uneseno ime brancha
		while (currentHead != NULL) {
			if (!_strcmpi(currentHead->commitPointer->branchName, lineBuffer)) {
				selectedCommit = currentHead->commitPointer;
				return selectedCommit;
			}

			currentHead = currentHead->nextHead;
		}

		//nije uneseno ime brancha, provjeri jeli ispravan commitID unesen
		if (sscanf(lineBuffer, "%d", &choice) != 1) {
			printf("Invalid input.\n");
			continue;
		}

		currentHead = heads;
		while (currentHead != NULL) {
			selectedCommit = FindCommit(currentHead->commitPointer, choice);
			if (selectedCommit != NULL)
				return selectedCommit;

			currentHead = currentHead->nextHead;
		}

		if (selectedCommit == NULL) {
			printf("Commit with ID [%d] not found.\n", choice);
		}


	} while (selectedCommit == NULL);
}

Commit MergeInputParse(Head heads) {
	Commit selectedCommit = NULL;
	Head currentHead = heads;
	char inputBuffer[BUFFER_SIZE];

	printf("Merge into which branch? Enter 0 to exit.\nInput: ");
	do {
		scanf("%s", inputBuffer);

		if (inputBuffer[0] == '\n')
			continue;
		else if (inputBuffer[0] == '0')
			return NULL;

		while (currentHead->commitPointer != NULL) {
			if (!_strcmpi(inputBuffer, currentHead->commitPointer->branchName)) {
				selectedCommit = currentHead->commitPointer;
				break;
			}

			currentHead = currentHead->nextHead;
		}

		currentHead = heads;
		if (selectedCommit == NULL) {
			printf("Specified branch does not exist. Try again or enter 0 to exit.\nInput: ");
			continue;
		}
		else
			break;

	} while (selectedCommit == NULL);



	return selectedCommit;
}

int ReplaceHeadInIndex(Commit newHead, Commit currentHead) {
	FILE* headsFile = NULL, * tempFile = NULL;
	char buffer[BUFFER_SIZE];

	headsFile = fopen("sample_repo/.git/.heads", "r");
	if (headsFile == NULL)
		return RETURN_WARNING_FILE_OPEN;

	tempFile = fopen("sample_repo/.git/.heads_temp", "w");
	if (tempFile == NULL) {
		fclose(headsFile);
		return RETURN_WARNING_FILE_OPEN;
	}

	while (fgets(buffer, BUFFER_SIZE, headsFile)) {
		if (strstr(buffer, currentHead->commitPath))
			fprintf(tempFile, "%s\n", newHead->commitPath);
		else
			fprintf(tempFile, "%s", buffer);
	}

	fclose(headsFile);
	fclose(tempFile);
	DeleteFileA("sample_repo/.git/.heads");
	rename("sample_repo/.git/.heads_temp", "sample_repo/.git/.heads");

	return RETURN_OK;
}

int AppendHeadToIndex(Commit newHead) {
	FILE* headsFile = NULL;

	headsFile = fopen("sample_repo/.git/.heads", "a");

	fprintf(headsFile, "\n%s", newHead->commitPath);

	fclose(headsFile);
	return RETURN_OK;
}

int ErrorReport(int errnum) {

	switch (errnum) {
	case RETURN_ERROR_MEM_ALLOC: {
		printf("\n\nERROR: Memory allocation.\n\n");
		break;
	}
	case RETURN_WARNING_FILE_OPEN: {
		printf("\n\nWARNING: Couldn't open file.\n\n");
		break;
	}
	case RETURN_WARNING_INVALID_FOREIGN_REFERENCE: {
		printf("\n\nWARNING: Invalid foreign reference.\n\n");
		break;
	}
	case RETURN_USER_ABORT: {
		printf("\n\nOperation canceled by the user.\n\n");
		break;
	}
	case RETURN_ERROR_COMMON_ANCESTOR: {
		printf("\n\nError: Couldn't find common ancestor for given commits.\n\n");
		break;
	}
	case RETURN_OK: {
		printf("\n\nSuccess.\n\n");
		break;
	}
	defaut: {
		printf("\n\nInvalid Error Code.\n\n");
		break;
	}
	}

	return RETURN_OK;
}

int PrintMenu(char* menu) {

	if (_strcmpi(menu, "main") == 0) {
		printf("\n\n1:Commit/Branch history.");
		printf("\n2:Commit files list.");
		printf("\n3:Checkout Commit/Branch");
		printf("\n4:Commit from Active Directory");
		printf("\n5:Branch from currently checked out commit");
		printf("\n6:Merge currently active commit");
		printf("\n7:Show currently active commit.");
		printf("\n0 to exit.\n");
		printf("Odabir: ");
		return RETURN_OK;
	}
	else
		return RETURN_OK;
}