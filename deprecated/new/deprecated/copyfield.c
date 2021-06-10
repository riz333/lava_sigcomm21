// copyfield.c -- created by Will Sussman on November 22, 2019

/*
A row is a string with fields separated by tab characters.
Rows are assumed to have one and only one newline character, at the end.
Takes in a row and a 1-indexed field number N.
On success, it returns a copy of the substring of row that is its Nth field.
On failure, it returns NULL.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "libwill.h"

char *copyfield(char *row, int fieldnum){
	if(row == NULL || fieldnum < 1){ //check for invalid inputs
		fprintf(stderr,"copyfield(): invalid input\n");
		return NULL;
	}

	int beg, end;
	bool begset = false;
	int tabs = 0;
	int i = 0;
	while(row[i] != '\n'){ //scan through row
		if(tabs == fieldnum && !begset){ //record beginning of field to copy
			beg = i;
			begset = true;
		}

		if(row[i] == '\t'){ //tab could mean beg or end of field to copy
			if(begset){
				end = i; //note: end is 1 more than the index of the end character
				break;
			}
			tabs++;
		}

		i++;
	}
	if(!begset){ //ran out of fields
		fprintf(stderr,"copyfield(): fieldnum DNE\n");
		return NULL;
	}

	//beg and end are now known
	return makesubstr(row,beg,end);
}
