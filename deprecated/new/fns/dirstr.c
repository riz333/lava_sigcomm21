// dirstr.c -- created by Will Sussman on December 21, 2019

#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

char *dirstr(struct Buffer buf){
	// printf("buf.contents has:\n");
	// for(int i = 0; i < buf.size; i++){
	// 	printf("%c",buf.contents[i]);
	// }
	// printf("\n");
	int size = 2 * buf.size; // (buf.size - 1) commas + 1 null terminal + buf.size chars
	// printf("path.len is %d\n",path.len);
	if(size == 0){ //edge case
		// printf("edge case\n");
		size++;
	}
	// printf("size is %d\n",size);
	// printf("pathstr(): init size == path.len is %d\n",size);
	// for(int i = 0; i < buf.size; i++){
	// 	// printf("current elt is %ld\n",path.elts[i]);
	// 	size += 1; //numdigits(path.elts[i]);
	// 	// printf("size is now %d\n",size);
	// }
	// printf("size is %d\n",size);
	char *str = malloc(sizeof(*str) * size);
	for(int i = 0; i < buf.size; i++){
		str[2 * i] = buf.contents[i];
		str[2 * i + 1] = ',';

		// printf("i iter is %d, j is %d\n",i,j);
		// printf("about to sprint %ld\n",path.elts[i]);
		// printf("j is currently %d\n",j);
		// sprintf(str + j,"%d",buf.contents[i]);
		// printf("just set str[%d] to %c\n",j,buf.contents[i]);
		// j += 1; //numdigits(path.elts[i]);
		// printf("j is now %d\n",j);
		// // printf("j is now %d\n",j);
		// printf("about to set str[%d] to \',\'\n",j);
		// str[j++] = ','; //last one will be overwritten with '\0'
		// printf("j is now %d\n",j);

		// printf("just added comma, j is now %d\n",j);
	}
	// printf("writing to str[size - 1 == %d]\n",size - 1);
	str[size - 1] = '\0';
	return str;
}
