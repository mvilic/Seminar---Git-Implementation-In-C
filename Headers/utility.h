#ifndef UTILITY_HEADER__
#define UTILITY_HEADER__

#include "common.h"

#define RETURN_EXIST  47
#define RETURN_NOEXIST -48
#define RETURN_NOTDIR -49

int SimulateCommandLine();
unsigned long Hash(const char*);
char* hash(char*);
int DirectoryExists(char*);
int ListDirectoryContents(const wchar_t* sDir);
char* DelimiterSlice(char*, char);
#endif