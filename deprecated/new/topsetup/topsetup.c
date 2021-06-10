// topsetup.c -- created by Will Sussman on December 20, 2019

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int main(int argc, char **argv){
	if(argc != 3){
		fprintf(stderr,"usage: topsetup ( -n | -a | -d ) topfile\n");
		return 1;
	}

	FILE *file;
	int c;
	char *line;
	size_t size;
	bool done;
	struct Entry entry;
	struct Buffer buf;
	struct List *list;
	if(!strcmp(argv[1],"-n")){
		//check that topfile does not already exist
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

		file = fopen(argv[2],"w"); //truncates if applicable
		if(file == NULL){
			perror(argv[2]);
			return 1;
		}

		list = NULL;
		while(1){
			printf("ID number: ");
			line = NULL;
			size = 0;
			getline(&line,&size,stdin);
			//(error check)
			entry.idval = atol(line);
			//(error check)
			line[strlen(line) - 1] = '\0'; //overwrite newline
			entry.id = line;
			// free(line);

			printf("IP address: ");
			line = NULL;
			size = 0;
			getline(&line,&size,stdin);
			//(error check)
			line[strlen(line) - 1] = '\0'; //overwrite newline
			entry.ipaddr = line;
			//(error check)
			//DO NOT FREE CURRENT LINE

			printf("Comma-separated ID numbers of neighbors (no whitespace): ");
			line = NULL;
			size = 0;
			getline(&line,&size,stdin);
			//(error check)
			buf.size = strlen(line); //double check this
			buf.contents = line;
			entry.fromvals = parsepath(buf,0); //libwill function
			//(error check)
			// free(line);
			line[strlen(line) - 1] = '\0'; //overwrite newline
			entry.from = line;

			// printf("Comma-separated 'to' ID numbers (no whitespace): ");
			// line = NULL;
			// size = 0;
			// getline(&line,&size,stdin);
			// //(error check)
			// buf.size = strlen(line); //double check this
			// buf.contents = line;
			// entry.tovals = parsepath(buf,0); //libwill function
			// //(error check)
			// // free(line);
			// line[strlen(line) - 1] = '\0'; //overwrite newline
			// entry.to = line;

			printf("Corresponding comma-separated directions of neighbors (lowercase): ");
			line = NULL;
			size = 0;
			getline(&line,&size,stdin);
			//(error check)
			buf.size = strlen(line); //double check this
			buf.contents = line;
			entry.dirchars = parsedirs(buf,0); //libwill function
			//(error check)
			// free(line)
			line[strlen(line) - 1] = '\0'; //overwrite newline
			entry.dirs = line;

			printf("Adding %ld\n",entry.idval);
			listbuild(entry,&list);
			//(error check)

			printf("Done? (y/n) ");
			while(1){
				c = getchar();
				while(getchar() != '\n'); //clear stdin
				if(c == 'y'){
					done = true;
					break;
				} else if(c == 'n'){
					done = false;
					break;
				} else {
					fprintf(stderr,"Invalid answer. Done? (y/n) ");
					//loop
				}
			}
			if(done){
				break;
			}
		}

		fprintlist(list,file);
		freelist(list);
		//(error check)
		fclose(file);
		//(error check)
		return 0;

	} else if(!strcmp(argv[1],"-a")){

		/* NOT NECESSARY -- opened for reading and closed by makelist() */
		// file = fopen(argv[2],"r");
		// if(file == NULL){
		// 	perror(argv[2]);
		// 	return 1;
		// }

		// buf = loadf(argv[2]);
		//(error check)
		list = NULL;
		if(makelist(argv[2],&list)){
			fprintf(stderr,"topsetup: makelist() failed\n");
			return 1;
		}
		//(error check)
		// free(buf.contents);

		file = fopen(argv[2],"w"); //truncates
		if(file == NULL){
			perror(argv[2]);
			return 1;
		}

		while(1){
			// printf("here\n");
			printf("ID number: ");
			line = NULL;
			size = 0;
			getline(&line,&size,stdin);
			//(error check)
			entry.idval = atol(line);
			//(error check)

			if(!entry.idval){
				fprintf(stderr,"ID number must be a positive integer\n");
				free(line);
			} else {
				//check that not already added
				if(listcheck(entry.idval,list)){ //no such idval -> true
					fprintf(stderr,"Already included\n");
					free(line);
				} else {
					line[strlen(line) - 1] = '\0'; //overwrite newline
					entry.id = line;
					// free(line);

					printf("IP address: ");
					line = NULL;
					size = 0;
					getline(&line,&size,stdin);
					//(error check)
					line[strlen(line) - 1] = '\0'; //overwrite newline
					entry.ipaddr = line;
					//(error check)
					//DO NOT FREE CURRENT LINE

					printf("Comma-separated ID numbers of neighbors (no whitespace): ");
					line = NULL;
					size = 0;
					getline(&line,&size,stdin);
					//(error check)
					buf.size = strlen(line); //double check this
					buf.contents = line;
					entry.fromvals = parsepath(buf,0); //libwill function
					//(error check)
					// free(line);
					line[strlen(line) - 1] = '\0'; //overwrite newline
					entry.from = line;

					printf("Corresponding comma-separated directions of neighbors (lowercase): ");
					line = NULL;
					size = 0;
					getline(&line,&size,stdin);
					//(error check)
					buf.size = strlen(line); //double check this
					buf.contents = line;
					entry.dirchars = parsedirs(buf,0); //libwill function
					//(error check)
					// free(line);
					line[strlen(line) - 1] = '\0'; //overwrite newline
					entry.dirs = line;

					// printf("Comma-separated 'to' ID numbers (no whitespace): ");
					// line = NULL;
					// size = 0;
					// getline(&line,&size,stdin);
					// //(error check)
					// buf.size = strlen(line); //double check this
					// buf.contents = line;
					// entry.tovals = parsepath(buf,0); //libwill function
					// //(error check)
					// // free(line);
					// line[strlen(line) - 1] = '\0'; //overwrite newline
					// entry.to = line;

					printf("Adding %ld\n",entry.idval);
					listadd(entry,&list);
					//(error check)
				}
			}

			printf("Done? (y/n) ");
			while(1){
				c = getchar();
				while(getchar() != '\n'); //clear stdin
				if(c == 'y'){
					done = true;
					break;
				} else if(c == 'n'){
					done = false;
					break;
				} else {
					fprintf(stderr,"Invalid answer. Done? (y/n) ");
					//loop
				}
			}
			if(done){
				// printf("topsetup() checkpoint\n");
				break;
			}
		}

		// printf("about to print list\n");
		fprintlist(list,file);
		// printf("about to free list\n");
		freelist(list);
		//(error check)
		fclose(file);
		//(error check)
		return 0;

	} else if(!strcmp(argv[1],"-d")){

		/* NOT NECESSARY -- opened for reading and closed by makelist() */
		// file = fopen(argv[2],"r");
		// if(file == NULL){
		// 	perror(argv[2]);
		// 	return 1;
		// }

		// buf = loadf(argv[2]);
		//(error check)
		list = NULL;
		// printf("topsetup about to makelist()\n");
		if(makelist(argv[2],&list)){
			fprintf(stderr,"topsetup: makelist() failed\n");
			return 1;
		}
		//(error check)
		// free(buf.contents);

		file = fopen(argv[2],"w"); //truncates
		if(file == NULL){
			perror(argv[2]);
			return 1;
		}

		while(1){
			printf("ID number: ");
			line = NULL;
			size = 0;
			getline(&line,&size,stdin);
			//(error check)

			// printf("topsetup about to listdelete(%ld)\n",atol(line));
			if(!atol(line)){
				fprintf(stderr,"ID number must be a positive integer\n");
			} else {
				printf("Deleting %ld\n",atol(line));
				listdelete(atol(line),&list);
				//(error check)
			}
			free(line);

			printf("Done? (y/n) ");
			while(1){
				c = getchar();
				while(getchar() != '\n'); //clear stdin
				if(c == 'y'){
					done = true;
					break;
				} else if(c == 'n'){
					done = false;
					break;
				} else {
					fprintf(stderr,"Invalid answer. Done? (y/n) ");
					//loop
				}
			}
			if(done){
				break;
			}
		}

		fprintlist(list,file);
		freelist(list);
		//(error check)
		fclose(file);
		//(error check)
		return 0;

	} else {
		fprintf(stderr,"usage: topsetup ( -n | -a | -d ) topfile\n");
		return 1;
	}

	// struct Buffer buf = loadf(argv[1]);
	// if(buf.size == -1){
	// 	fprintf(stderr,"loadf() failed\n");
	// 	return 1;
	// }
	// if(buf.size == 0){
	// 	fprintf(stderr,"%s is empty\n",argv[1]);
	// 	return 1;
	// }

	fprintf(stderr,"topsetup(): shouldn't reach here\n");
	return 1;
}
