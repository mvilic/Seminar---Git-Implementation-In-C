#ifndef HEADER_COMMON
#define HEADER_COMMON
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <io.h>
#include <wincrypt.h>
#include <stdbool.h>
#include <wchar.h>
#include <lmcons.h>

/*
###################################################
#			Constant Definitions				  #
###################################################
*/

#define BUFFER_SIZE 2048
#define MD5LEN 16

#define RETURN_OK 0
#define RETURN_ERROR_MEM_ALLOC -1
#define RETURN_WARNING_FILE_OPEN 1
#define RETURN_WARNING_INVALID_FOREIGN_REFERENCE 2
#define RETURN_USER_ABORT 10
#define RETURN_ERROR_COMMON_ANCESTOR -2

#endif