// topMain.c -- created by Will Sussman on December 27, 2019

#include <unistd.h>
#include <string.h>
#include "libtop.h"

int main(int argc, char **argv){
	if(argc != 3){
		fprintf(stderr,"usage: topMain ( -n | -a | -d ) topfile\n");
		return 1;
	}

	if(!strcmp(argv[1],"-n")){
		//check that topfile does not already exist
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
		FILE *topfile = fopen(argv[2],"w"); //truncates if applicable
		if(topfile == NULL){
			perror(argv[2]);
			return 1;
		}
		// struct Line *topdata = NULL, newline;
		// while(1){
		// 	printf("ID number: ");
		// 	line = NULL;
		// 	size = 0;
		// 	getline(&line,&size,stdin);
		// 	//(error check)
		// 	entry.idval = atol(line);
		// 	//(error check)
		// 	line[strlen(line) - 1] = '\0'; //overwrite newline
		// 	entry.id = line;
		// 	// free(line);

		// 	printf("IP address: ");
		// 	line = NULL;
		// 	size = 0;
		// 	getline(&line,&size,stdin);
		// 	//(error check)
		// 	line[strlen(line) - 1] = '\0'; //overwrite newline
		// 	entry.ipaddr = line;
		// 	//(error check)
		// 	//DO NOT FREE CURRENT LINE

		// 	printf("Comma-separated ID numbers of neighbors (no whitespace): ");
		// 	line = NULL;
		// 	size = 0;
		// 	getline(&line,&size,stdin);
		// 	//(error check)
		// 	buf.size = strlen(line); //double check this
		// 	buf.contents = line;
		// 	entry.fromvals = parsepath(buf,0); //libwill function
		// 	//(error check)
		// 	// free(line);
		// 	line[strlen(line) - 1] = '\0'; //overwrite newline
		// 	entry.from = line;

		// 	// printf("Comma-separated 'to' ID numbers (no whitespace): ");
		// 	// line = NULL;
		// 	// size = 0;
		// 	// getline(&line,&size,stdin);
		// 	// //(error check)
		// 	// buf.size = strlen(line); //double check this
		// 	// buf.contents = line;
		// 	// entry.tovals = parsepath(buf,0); //libwill function
		// 	// //(error check)
		// 	// // free(line);
		// 	// line[strlen(line) - 1] = '\0'; //overwrite newline
		// 	// entry.to = line;

		// 	printf("Corresponding comma-separated directions of neighbors (lowercase): ");
		// 	line = NULL;
		// 	size = 0;
		// 	getline(&line,&size,stdin);
		// 	//(error check)
		// 	buf.size = strlen(line); //double check this
		// 	buf.contents = line;
		// 	entry.dirchars = parsedirs(buf,0); //libwill function
		// 	//(error check)
		// 	// free(line)
		// 	line[strlen(line) - 1] = '\0'; //overwrite newline
		// 	entry.dirs = line;

		// 	printf("Adding %ld\n",entry.idval);
		// 	listbuild(entry,&list);
		// 	//(error check)

		// 	printf("Done? (y/n) ");
		// 	while(1){
		// 		c = getchar();
		// 		while(getchar() != '\n'); //clear stdin
		// 		if(c == 'y'){
		// 			done = true;
		// 			break;
		// 		} else if(c == 'n'){
		// 			done = false;
		// 			break;
		// 		} else {
		// 			fprintf(stderr,"Invalid answer. Done? (y/n) ");
		// 			//loop
		// 		}
		// 	}
		// 	if(done){
		// 		break;
		// 	}
		// }

		// fprintlist(list,file);
		// freelist(list);
		// //(error check)
		// fclose(file);
		// //(error check)
		// return 0;
	} else if(!strcmp(argv[1],"-a")){

	} else if(!strcmp(argv[1],"-d")){

	} else {
		fprintf(stderr,"usage: topMain ( -n | -a | -d ) topfile\n");
		return 1;
	}

	return 0;
}