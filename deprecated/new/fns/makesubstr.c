// makesubstr.c -- created by Will Sussman on November 22, 2019

/*
Takes in a string, a beginning index, and an end index
Makes a new string of size (end - beg + 1)
Copies string[beg] through string[end - 1] to new string
Sets end of new string to null character
Returns new string
On error, returns NULL
*/

#include <stdio.h>
#include <stdlib.h>

char *makesubstr(char *str, int beg, int end){
	if(str == NULL || end - beg <= 0 || beg < 0 || end < 0){ //check for invalid inputs
		fprintf(stderr,"makesubstr(): invalid input\n");
		return NULL;
	}

	char *newstr = malloc(sizeof(*newstr) * (end - beg + 1));
	if(newstr == NULL){
		fprintf(stderr,"makesubstr(): malloc() failed\n");
		return NULL;
	}
	for(int i = beg; i < end; i++){
		newstr[i - beg] = str[i];
	}
	newstr[end - beg] = '\0';
	return newstr;
}
