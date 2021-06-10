// listadd.c -- created by Will Sussman on December 20, 2019

#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int listadd(struct Entry entry, struct List **list){
	if(list == NULL){
		fprintf(stderr,"listadd(): list is NULL\n");
		return 1;
	}
	struct List *prevnode;
	struct List *currnode;
	// printf("size0: %ld\n",sizeof(struct List *));
	// printf("size0.1: %ld\n",sizeof(struct List **));
	// printf("size0.2: %ld\n",sizeof(struct Entry *));
	// printf("size0.3: %ld\n",sizeof(struct Entry **));
	if(*list == NULL){ //the list is empty
		// printf("size1: %ld\n",sizeof(**list));
		*list = malloc(sizeof(**list));
		//(error check)
		(*list)->entry = entry;
		(*list)->next = NULL;

		//no need to check other entries

		return 0;
	} else { //add newnode while making other nodes consistent
		prevnode = NULL;
		currnode = *list;
		while(currnode != NULL && entry.idval > currnode->entry.idval){
			//make consistent
			if(member(currnode->entry.idval,entry.fromvals)){
				// insert(entry.idval,&currnode->entry.fromvals);
				insert(entry,&currnode->entry);
				free(currnode->entry.from);
				free(currnode->entry.dirs);
				currnode->entry.from = pathstr(currnode->entry.fromvals);
				currnode->entry.dirs = dirstr(currnode->entry.dirchars);
			}
			// if(member(currnode->entry.idval,entry.tovals)){
			// 	insert(entry.idval,&currnode->entry.fromvals);
			// 	currnode->entry.from = pathstr(currnode->entry.fromvals);
			// }

			prevnode = currnode;
			currnode = currnode->next;
		}
		struct List *newnode = malloc(sizeof(*newnode));
		// printf("size2: %ld\n",sizeof(*newnode));
		newnode->entry = entry;
		newnode->next = currnode;
		if(prevnode == NULL){
			*list = newnode;
		} else {
			prevnode->next = newnode;
		}

		//continue making consistent
		prevnode = newnode;
		while(currnode != NULL){
			if(member(currnode->entry.idval,entry.fromvals)){
				// insert(entry.idval,&currnode->entry.fromvals);
				insert(entry,&currnode->entry);
				free(currnode->entry.from);
				free(currnode->entry.dirs);
				currnode->entry.from = pathstr(currnode->entry.fromvals);
				currnode->entry.dirs = dirstr(currnode->entry.dirchars);
			}
			// if(member(currnode->entry.idval,entry.tovals)){
			// 	insert(entry.idval,&currnode->entry.fromvals);
			// 	currnode->entry.from = pathstr(currnode->entry.fromvals);
			// }

			prevnode = currnode;
			currnode = currnode->next;
		}

		return 0;
	}

	fprintf(stderr,"listadd(): shouldn't reach here\n");
	return 1;
}
