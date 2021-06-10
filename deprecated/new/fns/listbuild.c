// listbuild.c -- created by Will Sussman on December 20, 2019

#include <stdio.h>
#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int listbuild(struct Entry entry, struct List **list){
	if(list == NULL){
		fprintf(stderr,"listbuild(): list is NULL\n");
		return 1;
	}
	struct List *prevnode;
	struct List *currnode;
	if(*list == NULL){ //the list is empty
		*list = malloc(sizeof(**list));
		//(error check)
		(*list)->entry = entry;
		(*list)->next = NULL;
		return 0;
	} else {
		prevnode = NULL;
		currnode = *list;
		while(currnode != NULL && entry.idval > currnode->entry.idval){
			prevnode = currnode;
			currnode = currnode->next;
		}
		struct List *newnode = malloc(sizeof(*newnode));
		newnode->entry = entry;
		newnode->next = currnode;
		if(prevnode == NULL){
			*list = newnode;
		} else {
			prevnode->next = newnode;
		}
		return 0;
	}

	fprintf(stderr,"listbuild(): shouldn't reach here\n");
	return 1;
}