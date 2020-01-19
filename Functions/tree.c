#include "tree.h"

FileNode CreateFileNode(char* path) {
	FileNode temp = NULL;
	temp = (FileNode)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	strcpy(temp->filePath, path);
	temp->fileHash = hash(path);
	temp->fileState = FILESTATE_COMMITTED;
	temp->nextFile = NULL;

	return temp;
}

FolderNode CreateFolderNode(char* path) {
	FolderNode temp = NULL;
	temp = (FolderNode)malloc(sizeof(*temp));

	if (temp == NULL)
		return NULL;

	strcpy(temp->folderPath, path);
	temp->fileTree = NULL;
	temp->nextFolder = NULL;

	return temp;
}

unsigned long Hash(const char* str)
{
	int key = 3;
	unsigned long hash = 0;
	FILE* file = NULL;
	char buffer[BUFFER_SIZE] = { 0 }, c = 0;

	file = fopen(str, "r");
	if (file == NULL)
		return RETURN_WARNING_FILE_OPEN;

	while ((c=getc(file))!=EOF) {
		hash = (hash + key) + (hash-c);
	}

	fclose(file);
	return hash;
}

DWORD hash(char* filePath)
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
	LPCWSTR filename[BUFFER_SIZE];
	char result[MD5LEN*2 + 1];
	int j = 0;

	mbsrtowcs(filename, &filePath, BUFFER_SIZE, NULL);

	hFile = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		dwStatus = GetLastError();
		printf("Error opening file %s\nError: %d\n", filename,
			dwStatus);
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