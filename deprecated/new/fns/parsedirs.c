// parsedirs.c -- created by Will Sussman on December 21, 2019

#include <stdio.h>
#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

struct Buffer parsedirs(struct Buffer buf, int cellstart){
	// printf("cellstart is %d\n",cellstart);
	// printf("parsedirs(): buf.contents is:\n%s",buf.contents);
	// printf("buf.contents[%d] is %c\n",cellstart,buf.contents[cellstart]);
	struct Buffer buf2;
	buf2.size = 1;
	int i = cellstart;
	while(i < buf.size && buf.contents[i] != '\t' && buf.contents[i] != '\n'){
		// printf("buf.contents[%d] is %c\n",i,buf.contents[i]);
		if(buf.contents[i] == ','){
			buf2.size++;
		}
		if(++i == buf.size){
			fprintf(stderr,"Cell not terminated\n");
			buf2.size = -1;
			buf2.contents = NULL;
			return buf2;
		}
	}

	buf2.contents = malloc(sizeof(*buf2.contents) * buf2.size);
	if(buf2.contents == NULL){
		perror("parsedirs(){malloc()}");
		buf2.size = -1;
		return buf2;
	}

	// char *ptr = buf.contents + cellstart - 1;
	// for(int j = 0; j < path.len; j++){
	// 	path.elts[j] = strtol(ptr + 1,&ptr,10); //Neat trick!
	// 	//could add error check here
	// }

	for(int j = 0; j < buf2.size; j++){
		// printf("buf.contents[%d] is %c, storing in buf2.contents[%d]\n",2*j + cellstart,buf.contents[2*j+cellstart],j);
		buf2.contents[j] = buf.contents[2 * j + cellstart]; //every other
		// printf("buf2.contents[%d] is now %c\n",j,buf2.contents[j]);
	}

	return buf2;
}