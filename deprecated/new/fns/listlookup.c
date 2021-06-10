// listlookup.c -- created by Will Sussman on December 20, 2019

#include "/home/riz3/willsLibrary/new/libwill.h"

int listlookup(long idval, struct List *list, struct Entry *entry){
	struct List *currnode = list;
	while(currnode != NULL && currnode->entry.idval < idval){
		currnode = currnode->next;
	}
	if(currnode == NULL){
		fprintf(stderr,"listlookup(): no such idval\n");
		return 1;
	}

	if(currnode->entry.idval == idval){
		*entry = currnode->entry;
		return 0;
	} else {
		return 1;
	}
	// *entry = currnode->entry;
	// return 0;
}