// topsetup.c -- created by Will Sussman on December 21, 2019

#include "/home/riz3/willsLibrary/new/libwill.h"

int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr,"usage: isconsistent topfile\n");
		return 1;
	}

	struct List *list = NULL;
	if(makelist(argv[1],&list)){
		fprintf(stderr,"isconsistent: makelist() failed\n");
		return 1;
	}

	struct List *currnode = list;
	struct Entry entry;
	while(currnode != NULL){
		if(currnode->entry.fromvals.len != currnode->entry.dirchars.size){
			fprintf(stderr,"%ld neighbor count and direction count mismatch\n",currnode->entry.idval);
			return 1;
		}
		for(int i = 0; i < currnode->entry.fromvals.len; i++){
			// printf("len is %d, currnode is %ld\n",currnode->entry.fromvals.len,currnode->entry.fromvals.elts[i]);
			if(listlookup(currnode->entry.fromvals.elts[i],list,&entry)){
				fprintf(stderr,"No entry for %ld (neighbor of %ld)\n",currnode->entry.fromvals.elts[i],currnode->entry.idval);
				return 1;
			}
			if(!member(currnode->entry.idval,entry.fromvals)){
				fprintf(stderr,"%ld missing from neighbors of %ld\n",currnode->entry.idval,entry.idval);
				return 1;
			}
		}
		currnode = currnode->next;
	}
	printf("Topology is consistent\n");
	return 0;
}
