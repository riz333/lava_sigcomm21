// delete.c -- created by Will Sussman on December 20, 2019

#include <stdlib.h>
// #include <string.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int delete(long idval, /*struct Path *path*/struct Entry *entry){
	// if(path == NULL){
	// 	fprintf(stderr,"insert(): path is NULL\n");
	// 	return 1;
	// }

	long int *newelts = malloc(sizeof(*newelts) * --(entry->fromvals.len));
	char *newdirs = malloc(sizeof(*newdirs) * --(entry->dirchars.size));
	int i = 0;
	while((entry->fromvals.elts)[i] < idval){
		newelts[i] = (entry->fromvals.elts)[i];
		newdirs[i] = (entry->dirchars.contents)[i];
		i++;
	}
	// newelts[i++] = idval;
	while(i < entry->fromvals.len){
		newelts[i] = (entry->fromvals.elts)[i + 1];
		newdirs[i] = (entry->dirchars.contents)[i + 1];
		i++;
	}
	free(entry->fromvals.elts);
	entry->fromvals.elts = newelts;
	free(entry->dirchars.contents);
	entry->dirchars.contents = newdirs;
	return 0;
}