/* getlineid.c -- created by Will Sussman on June 20, 2019 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include "liblawa.h"

/* Takes in a file pointer and an ID number
 * The file should have already been opened for reading
 * Moves to the top of the file
 * Repeatedly gets a line and checks its initial number against the ID number
 * If a match, a copy of the line is returned
 * Otherwise NULL is returned */
char *getlineid(FILE *file, int idnum){
	if(file == NULL){
		fprintf(stderr,"getlineid(): file is NULL\n");
		return NULL;
	}

	rewind(file);
	char *line = NULL;
	size_t size = 0;
	while(getline(&line,&size,file) != -1){
		if(strtol(line,NULL,10) == idnum){
			return line;
		}
		resetptr(line);
		size = 0;
	}
	resetptr(line);
	fprintf(stderr,"getlineid(): could not find line %d\n",idnum);
	return NULL;
}
