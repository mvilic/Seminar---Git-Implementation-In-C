#include "common.h"
#include "utility.h"

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
	char result[MD5LEN * 2 + 1];
	int j = 0;

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

	//Specify a file mask. *.*  everything 
	wsprintf(sPath, L"%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return RETURN_NOEXIST;
	}

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
		{ 
			wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

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