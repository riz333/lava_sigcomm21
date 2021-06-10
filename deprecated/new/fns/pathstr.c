// pathstr.c -- created by Will Sussman on December 20, 2019

#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

char *pathstr(struct Path path){
	int size = path.len; // (path.len - 1) commas + 1 null terminal
	// printf("path.len is %d\n",path.len);
	if(size == 0){ //edge case
		// printf("edge case\n");
		size++;
	}
	// printf("pathstr(): init size == path.len is %d\n",size);
	for(int i = 0; i < path.len; i++){
		// printf("current elt is %ld\n",path.elts[i]);
		size += numdigits(path.elts[i]);
		// printf("size is now %d\n",size);
	}
	// printf("size is %d\n",size);
	char *str = malloc(sizeof(*str) * size);
	int j = 0;
	for(int i = 0; i < path.len; i++){
		// printf("i iter is %d, j is %d\n",i,j);
		// printf("about to sprint %ld\n",path.elts[i]);
		sprintf(str + j,"%ld",path.elts[i]);
		j += numdigits(path.elts[i]);
		// printf("j is now %d\n",j);
		str[j++] = ','; //last one will be overwritten with '\0'
		// printf("just added comma, j is now %d\n",j);
	}
	// printf("writing to str[size - 1 == %d]\n",size - 1);
	str[size - 1] = '\0';
	return str;
}
