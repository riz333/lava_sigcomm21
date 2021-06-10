// prepbytes.c -- created by Will Sussman on December 21, 2019

#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int prepbytes(struct Path path, struct Buffer *buf, struct List *list){
	if(path.len < 2){
		fprintf(stderr,"Path too short\n");
		return 1;
	}

	buf->size = 2 * path.len - 2; //I&O for middle elts, I|O for end elts
	buf->contents = malloc(sizeof(*buf->contents) * buf->size);

	long lval, rval;
	struct Entry entry;
	for(int i = 0; i < path.len - 1; i++){
		// printf("prepbytes() loop\n");
		lval = path.elts[i];
		rval = path.elts[i+1];
		listlookup(lval,list,&entry);
		buf->contents[2 * i] = entry.dirchars.contents[memind(rval,entry.fromvals)];
		listlookup(rval,list,&entry);
		buf->contents[2 * i + 1] = entry.dirchars.contents[memind(lval,entry.fromvals)];
	}
	// printf("prepbytes() about to return\n");
	return 0;
}