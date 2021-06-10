// freelist.c -- created by Will Sussman on December 20, 2019

#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int freelist(struct List *list){
	// if(list == NULL){
	// 	return 0; //already done
	// }

	struct List *currnode = list;
	struct List *nextnode;
	// struct Entry entry;
	while(currnode != NULL){
		// entry = currnode->entry;
		nextnode = currnode->next;
		// free(entry.id);
		// free(entry.ipaddr);
		// free(entry.from);
		// free(entry.fromvals.elts);
		// free(entry.to);
		// free(entry.tovals.elts);
		freenode(currnode);
		currnode = nextnode;
	}
	return 0;
}