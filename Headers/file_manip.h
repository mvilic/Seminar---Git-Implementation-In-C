#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>


#define RETURN_EXIST  47
#define RETURN_NOEXIST -48
#define RETURN_NOTDIR -49

int DirectoryExists(char*);

int ListDirectoryContents(const wchar_t*);
int RemoveDirectoryFull(const wchar_t*);

