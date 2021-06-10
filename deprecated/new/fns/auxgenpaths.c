// auxgenpaths.c -- created by Will Sussman on December 21, 2019

#include "/home/riz3/willsLibrary/new/libwill.h"

int auxgenpaths(struct Path *path, struct List *list, FILE *destfile, long startid, long endid){
	// printf("in auxgenpaths, startid is %ld\n",startid);
	struct Entry root;
	listlookup(startid,list,&root);
	append(root.idval,path);

	if(root.idval == endid){
		// fprintf(stdout,"printing: %s\n",pathstr(*path));
		fprintf(destfile,"%s\n",pathstr(*path));
		// printf("returning\n");
		unappend(root.idval,path);
		return 0;
	}

	for(int i = 0; i < root.fromvals.len; i++){
		// printf("root.fromvals.elts[%d] is %ld\n",i,root.fromvals.elts[i]);
		long currelt = root.fromvals.elts[i];

		if(!member(currelt,*path)){
			auxgenpaths(path,list,destfile,currelt,endid);
		}

		// if(currelt == endid){
		// 	fprintf(stdout,"printing: %s\n",pathstr(*path));
		// 	fprintf(destfile,"%s\n",pathstr(*path));
		// } else {
		// 	if(!member(currelt,*path)){
		// 		auxgenpaths(path,list,destfile,currelt,endid);
		// 	}
		// }

	}

	// printf("returning\n");
	unappend(root.idval,path);
	return 0;
}