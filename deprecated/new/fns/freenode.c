// freenode.c -- created by Will Sussman on December 20, 2019

#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int freenode(struct List *node){
	// printf("in freenode()\n");
	if(node == NULL){
		fprintf(stderr,"freenode(): node is NULL\n");
		return 1;
	}

	struct Entry entry = node->entry;
	// printf("entry.id is: %s (just in case, entry.idval is: %ld)\n",entry.id,entry.idval);
	free(entry.id);
	free(entry.ipaddr);
	free(entry.from);
	free(entry.dirs);
	// printf("about to free elts\n");
	// for(int i = 0; i < entry.fromvals.len; i++){
	// 	printf("elt#%d: %ld\n",i,entry.fromvals.elts[i]);
	// }
	if(entry.fromvals.elts != NULL){
		free(entry.fromvals.elts);
	}
	if(entry.dirchars.contents != NULL){
		free(entry.dirchars.contents);
	}
	// free(entry.to);
	// free(entry.tovals.elts);
	free(node);
	return 0;
}
