// listcheck.c -- created by Will Sussman on December 21, 2019

#include "/home/riz3/willsLibrary/new/libwill.h"

bool listcheck(long idval, struct List *list){
	struct List *currnode = list;
	while(currnode != NULL && currnode->entry.idval < idval){
		currnode = currnode->next;
	}
	if(currnode == NULL){
		// fprintf(stderr,"listlookup(): no such idval\n");
		return false;
	}

	if(currnode->entry.idval == idval){
		return true;
	} else {
		return false;
	}
	// *entry = currnode->entry;
	// return true;
}
