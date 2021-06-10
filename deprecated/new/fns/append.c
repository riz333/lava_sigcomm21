// append.c -- created by Will Sussman on December 21, 2019

#include <stdlib.h>
#include <string.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int append(long idval, struct Path *path){
	if(path == NULL){
		fprintf(stderr,"append(): path is NULL\n");
		return 1;
	}

	path->elts = realloc(path->elts,sizeof(*path->elts) * ++path->len);
	path->elts[path->len - 1] = idval;

	/*
	long int *newelts = malloc(sizeof(*newelts) * ++path->len);
	int i = 0;
	// printf("path->len is %d, idval is %ld\n",path->len,idval);
	while(i < (path->len - 1) && path->elts[i] < idval){
		// printf("i is currently: %d\n",i);
		// printf("...so path->elts[i] is: %ld\n",path->elts[i]);
		newelts[i] = path->elts[i];
		i++;
		// printf("i is now: %d\n",i);
	}
	newelts[i++] = idval;
	while(i < path->len){
		newelts[i] = path->elts[i - 1];
		i++;
	}
	free(path->elts);
	path->elts = newelts;
	*/

	return 0;
}
