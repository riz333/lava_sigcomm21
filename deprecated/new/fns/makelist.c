// makelist.c -- created by Will Sussman on December 20, 2019

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int makelist(char *filename, struct List **list){
	struct Entry entry;
	struct Buffer buf;

	FILE *file = fopen(filename,"r"); //open file
	if(file == NULL){
		perror(filename);
		return 1;
	}

	char *line = NULL;
	size_t size = 0;
	while(getline(&line,&size,file) != -1){
		// printf("\nmakelist(): line is: %s",line);
		//line has: id ipaddr from to
		buf.size = strlen(line); //double check this
		buf.contents = line;
		// printf("buf.contents is now %s\n",buf.contents);

		entry.idval = atol(line);
		//(error check)
		// printf("entry.idval is: %ld\n",entry.idval);
		entry.id = malloc(sizeof(*entry.id) * (numdigits(entry.idval) + 1));
		sprintf(entry.id,"%ld",entry.idval);

		// printf("about to print buf\n");
		// for(int i = 0; i < buf.size; i++){
		// 	printf("%c",buf.contents[i]);
		// }
		// printf("done printing buf\n");
		// printf("cellstart(1,2,buf) is %d, cellstart(1,3,buf) is %d\n",cellstart(1,2,buf),cellstart(1,3,buf));
		if(cellstart(1,2,buf) == cellstart(1,3,buf) - 1){
			entry.ipaddr = "";
		} else {
			entry.ipaddr = makesubstr(buf.contents,cellstart(1,2,buf),cellstart(1,3,buf) - 1);
		}
		//(error check)
		// printf("entry.ipaddr is: %s\n",entry.ipaddr);
		if(buf.contents[cellstart(1,3,buf)] == '\t' || buf.contents[cellstart(1,3,buf)] == '\n'){
			entry.fromvals.len = 0;
			entry.fromvals.elts = NULL;
		} else {
			entry.fromvals = parsepath(buf,cellstart(1,3,buf));
		}
		// printf("entry.fromvals.len is %d\n",entry.fromvals.len);
		entry.from = pathstr(entry.fromvals);
		// printf("entry.from is now %s\n",entry.from);
		//(error check)
		// for(int i = 0; i < entry.fromvals.len; i++){
		// 	printf("entry.fromvals[%d] is %ld\n",i,entry.fromvals.elts[i]);
		// }

		if(buf.contents[cellstart(1,4,buf)] == '\t' || buf.contents[cellstart(1,4,buf)] == '\n'){
			entry.dirchars.size = 0;
			entry.dirchars.contents = NULL;
		} else {
			entry.dirchars = parsedirs(buf,cellstart(1,4,buf));
		}
		// printf("entry.fromvals.len is %d\n",entry.fromvals.len);
		entry.dirs = dirstr(entry.dirchars);

		entry.socket = -1;

		free(buf.contents);

		// entry.tovals = parsepath(buf,cellstart(1,4,buf));
		//(error check)

		// printf("makelist(): about to build list\n");
		listbuild(entry,list);
		//(error check)

		line = NULL;
		size = 0;
	}

	if(line != NULL){
		free(line);
	}

	fclose(file);
	//(error check)
	return 0;
}
