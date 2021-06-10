// loadf.c -- created by Will Sussman on December 10, 2019

/*
Takes in a filename, opens the file, determines the file size, allocates memory
for the buffer, reads the file into the buffer, and closes the file
On success, buf.contents points to the new memory (may be NULL if the file size was 0),
buf.size is set, and buf is returned
On error, buf.contents is NULL, buf.size is -1, and buf is returned
*/

#include <stdio.h>
#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

struct Buffer loadf(char *filename){
	struct Buffer buf;

	FILE *file = fopen(filename,"r"); //open file
	if(file == NULL){
		// fprintf(stderr,"fopen() failed\n");
		perror(filename);
		buf.contents = NULL;
		buf.size = -1;
		return buf;
	}

	if(fseek(file,0,SEEK_END)){ //move to end of file
		// fprintf(stderr,"fseek() failed\n");
		perror(filename);
		buf.contents = NULL;
		buf.size = -1;
		return buf;
	}

	buf.size = ftell(file); //read file position
	if(buf.size == -1){
		// fprintf(stderr,"ftell() failed\n");
		perror(filename);
		buf.contents = NULL;
		return buf;
	}

	if(fseek(file,0,SEEK_SET)){ //move back to beg of file
		// fprintf(stderr,"fseek() failed\n");
		perror(filename);
		buf.contents = NULL;
		buf.size = -1;
		return buf;
	}

	buf.contents = malloc(sizeof(char) * buf.size); //allocate buffer
	if(buf.contents == NULL && buf.size != 0){
		// fprintf(stderr,"malloc() failed\n");
		perror("loadf(){malloc()}");
		buf.size = -1;
		return buf;
	}

	if(fread(buf.contents,sizeof(char),buf.size,file) < buf.size){ //read file into buffer
		fprintf(stderr,"fread() failed\n");
		buf.contents = NULL;
		buf.size = -1;
		return buf;
	}

	if(fclose(file)){ //close file
		// fprintf(stderr,"fclose() failed\n");
		perror(filename);
		buf.contents = NULL;
		buf.size = -1;
		return buf;
	}

	return buf;
}
