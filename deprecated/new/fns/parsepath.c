// parsepath.c -- created by Will Sussman on December 10, 2019

/*
Takes in a buffer and the index of the beginning of the cell to parse
Cell should contain comma-separated integers and end with tab or newline
Commas must immediately follow integers (nothing in between)
Does not check for strtol() errors
On success, path.len and path.elts set and path is returned
On failure, path.len is -1, path.elts is NULL, and path is returned
*/

#include <stdio.h>
#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

struct Path parsepath(struct Buffer buf, int cellstart){
	struct Path path;
	path.len = 1;
	int i = cellstart;
	while(i < buf.size && buf.contents[i] != '\t' && buf.contents[i] != '\n'){
		if(buf.contents[i] == ','){
			path.len++;
		}
		if(++i == buf.size){
			fprintf(stderr,"Cell not terminated\n");
			path.len = -1;
			path.elts = NULL;
			return path;
		}
	}

	path.elts = malloc(sizeof(*path.elts) * path.len);
	if(path.elts == NULL){
		perror("parsepath(){malloc()}");
		path.len = -1;
		return path;
	}

	char *ptr = buf.contents + cellstart - 1;
	for(int j = 0; j < path.len; j++){
		path.elts[j] = strtol(ptr + 1,&ptr,10); //Neat trick!
		//could add error check here
	}

	return path;
}
