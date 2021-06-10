// cellstart.c -- created by Will Sussman on November 22, 2019

/*
Takes in a buffer of newline-terminated rows with tab-separated fields
Rows and columns are one-indexed
On success, returns offset from beginning of buffer
On failure, returns -1
*/

#include <stdio.h>
#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int cellstart(int row, int col, struct Buffer buf){
	// printf("cellstart: row is %d, col is %d, buf.contents is:\n%s",row,col,buf.contents);
	if(buf.contents == NULL){
		fprintf(stderr,"cellstart(): buf.contents is NULL\n");
		return -1;
	}
	if(row < 1){
		fprintf(stderr,"cellstart(): row is < 1\n");
		return -1;
	}
	if(col < 1){
		fprintf(stderr,"cellstart(): col is < 1\n");
		return -1;
	}
	if(buf.size < 0){
		fprintf(stderr,"cellstart(): buf.size is < 0\n");
		return -1;
	}

	int i = 0, newlines = 0, tabs = 0;
	while(newlines != row - 1 && i < buf.size){
		// printf("i is currently %d\n",i);
		if(buf.contents[i++] == '\n'){
			// printf("new row\n");
			newlines++;
		}
	}
	// printf("\tmiddle i is %d\n",i);

	while(tabs != col - 1 && i < buf.size){
		// printf("i is currently %d\n",i);
		if(buf.contents[i] == '\n'){
			fprintf(stderr,"cellstart(): col DNE\n");
			return -1;
		}
		if(buf.contents[i++] == '\t'){
			// printf("new tab\n");
			tabs++;
		}
	}

	if(i == buf.size){
		fprintf(stderr,"cellstart(): premature EOF\n");
		return -1;
	}

	// printf("returning i == %d\n",i);
	// printf("buf.contents[i] is %c\n",buf.contents[i]);
	return i;
}
