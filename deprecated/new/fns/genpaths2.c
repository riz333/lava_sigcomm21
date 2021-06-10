// genpaths2.c -- created by Will Sussman on December 21, 2019

#include <stdlib.h>
// #include <string.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int genpaths2(struct Path *path, struct List *list, struct Path **pathset, long startid, long endid, int *numpaths){
	struct Entry root;
	listlookup(startid,list,&root);
	append(root.idval,path);
	// printf("*numpaths is currently: %d\n",*numpaths);
	// printf("root is %ld\n",root.idval);
	// printf("path is now:\n");
	// for(int i = 0; i < path->len; i++){
	// 	printf("%ld,",path->elts[i]);
	// }
	// printf("\n");

	if(root.idval == endid){
		*pathset = realloc(*pathset,sizeof(**pathset) * ++*numpaths);
		// printf("*numpaths is now: %d\n",*numpaths);
		// printf("size is: %ld\n",sizeof(*pathset[*numpaths - 1]));
		// memcpy(pathset[*numpaths - 1],path,sizeof(*path));
		pathset[*numpaths - 1] = malloc(sizeof(*pathset[*numpaths - 1]));
		pathset[*numpaths - 1]->len = path->len;
		// printf("path->len is %d\n",path->len);
		// printf("pathset[*numpaths - 1]->len is %d\n",pathset[*numpaths - 1]->len);
		pathset[*numpaths - 1]->elts = malloc(sizeof(*pathset[*numpaths - 1]->elts) * path->len);
		for(int i = 0; i < path->len; i++){
			// printf("path->elts[i] is: %ld\n",path->elts[i]);
			pathset[*numpaths - 1]->elts[i] = path->elts[i];
		}
		unappend(root.idval,path);
		// printf("returning\n");
		return 0;
	}

	for(int i = 0; i < root.fromvals.len; i++){
		long currelt = root.fromvals.elts[i];
		if(!member(currelt,*path)){
			genpaths2(path,list,pathset,currelt,endid,numpaths);
		} else {
			// printf("skipping\n");
		}
	}

	unappend(root.idval,path);
	// printf("returning\n");
	return 0;
}
