// listdelete.c -- created by Will Sussman on December 20, 2019

#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int listdelete(long idval, struct List **list){
	if(list == NULL){
		fprintf(stderr,"listdelete(): list is NULL\n");
		return 1;
	}
	struct List *prevnode;
	struct List *currnode;
	struct Entry target;
	if(listlookup(idval,*list,&target)){
		return 1;
	}
	// printf("target is %ld\n",target.idval);
	//(error check)
	
	//delete idval while making other nodes consistent
	prevnode = NULL;
	currnode = *list;
	while(currnode != NULL && target.idval > currnode->entry.idval){
		// printf("currnode is %ld\n",currnode->entry.idval);
		//make consistent

		// if(member(currnode->entry.idval,target.fromvals)){
		// 	delete(target.idval,&currnode->entry.fromvals);
		// 	free(currnode->entry.from);
		// 	currnode->entry.from = pathstr(currnode->entry.fromvals);
		// }
		if(member(target.idval,currnode->entry.fromvals)){
			delete(target.idval,&currnode->entry);
			free(currnode->entry.from);
			free(currnode->entry.dirs);
			currnode->entry.from = pathstr(currnode->entry.fromvals);
			currnode->entry.dirs = dirstr(currnode->entry.dirchars);
		}

		// if(member(currnode->entry.idval,entry.tovals)){
		// 	delete(entry.idval,&currnode->entry.fromvals);
		// 	currnode->entry.from = pathstr(currnode->entry.fromvals);
		// }

		prevnode = currnode;
		currnode = currnode->next;
		// printf("currnode is now %ld\n",currnode->entry.idval);
	}
	// struct List *newnode = malloc(sizeof(*newnode));
	// newnode->entry = entry;
	// newnode->next = currnode;
	// if(prevnode == NULL){
	// 	*list = newnode;
	// } else {
	// 	prevnode->next = newnode;
	// }

	// if(target.idval > currnode->entry.idval){
	// 	printf("hint\n");
	// } else {
	// 	printf("opp hint\n");
	// }

	if(currnode == NULL){
		fprintf(stderr,"listdelete(): no such entry\n");
		return 1;
	}
	if(prevnode != NULL){
		prevnode->next = currnode->next;
	} else {
		*list = currnode->next;
	}
	// printf("idval is %ld, about to free node with entry->idval %ld\n",idval,currnode->entry.idval);
	freenode(currnode);
	if(prevnode != NULL){
		currnode = prevnode->next;
	} else {
		currnode = *list;
	}
	if(currnode == NULL){
		// printf("currnode is NOW null\n");
	}
	// printf("listdelete(): currnode now has entry idval %ld\n",currnode->entry.id);
	// printf("currnod'es new num neighbors: %d\n",currnode->entry.fromvals.len);
	// printf("currnode's new fromvals:\n");
	// for(int i = 0; i < currnode->entry.fromvals.len; i++){
	// 	printf("--elt#%d: %ld\n",i,currnode->entry.fromvals.elts[i]);
	// }

	//continue making consistent
	// prevnode = newnode;
	while(currnode != NULL){
		// printf("listdelete() about to call member() on currnode (#%ld), and target (#%ld) neighbors\n",currnode->entry.idval,target.idval);

		// if(member(currnode->entry.idval,target.fromvals)){
		// 	delete(target.idval,&currnode->entry.fromvals);
		// 	free(currnode->entry.from);
		// 	currnode->entry.from = pathstr(currnode->entry.fromvals);
		// }
		if(member(target.idval,currnode->entry.fromvals)){
			delete(target.idval,&currnode->entry);
			free(currnode->entry.from);
			free(currnode->entry.dirs);
			currnode->entry.from = pathstr(currnode->entry.fromvals);
			currnode->entry.dirs = dirstr(currnode->entry.dirchars);
		}


		// if(member(currnode->entry.idval,entry.tovals)){
		// 	delete(entry.idval,&currnode->entry.fromvals);
		// 	currnode->entry.from = pathstr(currnode->entry.fromvals);
		// }


		prevnode = currnode;
		currnode = currnode->next;
	}

	return 0;
}
