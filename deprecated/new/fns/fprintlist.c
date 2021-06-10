// fprintlist.c -- created by Will Sussman on December 20, 2019

#include <stdio.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int fprintlist(struct List *list, FILE *file){
	if(file == NULL){
		fprintf(stderr,"fprintlist(): file is NULL\n");
		return 1;
	}

	struct List *currnode = list;
	struct Entry entry;
	while(currnode != NULL){
		entry = currnode->entry;
		// fprintf(stdout,"%s\t%s\t%s\t%s\n",entry.id,entry.ipaddr,entry.from,entry.to);
		fprintf(file,"%s\t%s\t%s\t%s\n",entry.id,entry.ipaddr,entry.from,entry.dirs);
		currnode = currnode->next;
	}

	return 0;
}