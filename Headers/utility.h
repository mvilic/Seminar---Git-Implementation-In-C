#ifndef UTILITY_HEADER__
#define UTILITY_HEADER__

#include "common.h"
#include "commit.h"
#include "tree.h"

/*
###################################################
#			Constant Definitions				  #
###################################################
*/

#define RETURN_EXIST  47
#define RETURN_NOEXIST -48
#define RETURN_NOTDIR -49

/*
###################################################
#				General Functions				  #
###################################################
*/

//Computes an MD5 hash of the file at the given path
char* hash(char* pathToFile);

//Checks if given directory exists.
int DirectoryExists(char*);

//Lists all files and folders of a given directory
int ListDirectoryContents(const wchar_t*);

//Returns a new string starting after the first occurence of the delimiter in the passed string
char* DelimiterSlice(char* string, char delimiter);

int ErrorReport(int errnum);

int PrintMenu(char* menu);

/*
###################################################
#				User Interaction				  #
###################################################
*/

//Yes/No choice parser
int Choice(); 

//Numeric choice parsers with lower and upper limiters
int Option(int lowerLimit, int upperLimit);

//User input parsing for the Checkout command
Commit CheckoutInputParse(Head heads);

//User input parsing for the Merge command
Commit MergeInputParse(Head heads);

/*
###################################################
#				Index Operations				  #
###################################################
*/

//Replaces the current head of a branch with the passed commit in the index file
int ReplaceHeadInIndex(Commit newHead, Commit currentHead);

//Appends a new head in the index file
int AppendHeadToIndex(Commit newHead);


#endif