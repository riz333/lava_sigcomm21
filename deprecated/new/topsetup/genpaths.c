// genpaths.c -- created by Will Sussman on December 21, 2019

#include <stdlib.h>
#include <unistd.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int main(int argc, char **argv){
	if(argc != 5){
		fprintf(stderr,"usage: genpaths topfile destfile startid endid\n");
		return 1;
	}

	struct List *list = NULL;
	makelist(argv[1],&list);

	long startid = atol(argv[3]);
	long endid = atol(argv[4]);

	// struct Entry start;
	// listlookup(atol(startid),list,start);

	//check that destfile does not already exist
	int c;
	if(!access(argv[2],F_OK)){
		fprintf(stderr,"File named \'%s\' already exists. Overwrite? (y/n) ",argv[2]);
		while(1){
			c = getchar();
			while(getchar() != '\n'); //clear stdin
			if(c == 'y'){
				break;
			} else if(c == 'n'){
				return 0;
			} else {
				fprintf(stderr,"Invalid answer. Overwrite file named \'%s\'? (y/n) ",argv[2]);
				//loop
			}
		}
	}

	FILE *destfile = fopen(argv[2],"w"); //truncates if applicable
	if(destfile == NULL){
		perror(argv[2]);
		return 1;
	}

	struct Path path;
	path.len = 0;
	path.elts = NULL;
	auxgenpaths(&path,list,destfile,startid,endid);

	fclose(destfile);
	return 0;
}