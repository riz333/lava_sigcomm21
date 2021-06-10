// insert.c -- created by Will Sussman on December 20, 2019

#include <stdlib.h>
#include <string.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int insert(/*long idval, struct Path *path*/struct Entry src, struct Entry *dest){
	// printf("in insert\n");
	// if(path == NULL){
	// 	fprintf(stderr,"insert(): path is NULL\n");
	// 	return 1;
	// }

	long int *newelts = malloc(sizeof(*newelts) * ++(dest->fromvals.len));
	char *newdirs = malloc(sizeof(*newdirs) * ++(dest->dirchars.size));
	int i = 0;
	// printf("path->len is %d, idval is %ld\n",path->len,idval);
	while(i < (dest->fromvals.len - 1) && (dest->fromvals.elts)[i] < src.idval){
		// printf("i is currently: %d\n",i);
		// printf("...so path->elts[i] is: %ld\n",path->elts[i]);
		newelts[i] = (dest->fromvals.elts)[i];
		newdirs[i] = (dest->dirchars.contents)[i];
		i++;
		// printf("i is now: %d\n",i);
	}
	
	char dir;
	char oppdir = (src.dirchars.contents)[memind(dest->idval,src.fromvals)];
	if(oppdir == 'n'){
		dir = 's';
	} else if(oppdir == 'e'){
		dir = 'w';
	} else if(oppdir == 's'){
		dir = 'n';
	} else if(oppdir == 'w'){
		dir = 'e';
	}
	// printf("oppdir is %c, dir is %c\n",oppdir,dir);
	newdirs[i] = dir;
	newelts[i++] = src.idval;
	while(i < dest->fromvals.len){
		newelts[i] = (dest->fromvals.elts)[i - 1];
		newdirs[i] = (dest->dirchars.contents)[i - 1];
		i++;
	}
	free(dest->fromvals.elts);
	free(dest->dirchars.contents);
	dest->fromvals.elts = newelts;
	dest->dirchars.contents = newdirs;
	return 0;
}
