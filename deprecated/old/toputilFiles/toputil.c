/* Created by Will Sussman on June 6, 2019
 * toputil is short for topology utility
 * usage: toputil -r|-w|-d filename number
 * gcc toputil.c -g3 -std=c99 -pedantic -Wall -o toputil */

#define _GNU_SOURCE
#define EPRINT(string) {fprintf(stderr,string); exit(0);}
#define RESETSTR(string) {if(string != NULL){free(string); string = NULL;}}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isnonnegint(char *string);

int main(int argc, char **argv){
	// "./toputil" would set argc to 1 and argv[0] to "./toputil"

	if(argc != 4){ //check that all 4 arguments were entered
		EPRINT("usage: toputil -r|-w|-d filename number\n");
	}

	//store numeric value of the number argument
	int number;
	if(isnonnegint(argv[3])){
		number = atoi(argv[3]);
		if(number == 0){
			EPRINT("number must be a positive integer.\n");
		}
	} else {
		EPRINT("number must be a positive integer.\n");
	}

	//open the topology file
	FILE *fpr = fopen(argv[2],"r");
	if(fpr == NULL){
		EPRINT("Could not open topology file.\n");
	}

	bool w;
	char *line = NULL;
	size_t size = 0;
	if(!strcmp(argv[1],"-r")){ //read
		bool printed = false;
		line = NULL; //redundant
		size = 0; //redundant
		while(getline(&line,&size,fpr) != -1){ //find and print line #number
			if(strtol(line,NULL,10) == number){
				printf("%s",line);
				printed = true;
			}
			RESETSTR(line);
			size = 0;
		}
		if(!printed){
			EPRINT("No such entry.\n");
		}
	} else if((w = !strcmp(argv[1],"-w")) || !strcmp(argv[1],"-d")){ //write or delete
		int cluster;
		int pin;
		int length;
		if(w){ //read in information to be written
			printf("Enter cluster number: ");
			line = NULL; //redundant
			size = 0; //redundant
			if((length = getline(&line,&size,stdin)) == -1){
				EPRINT("\nCould not read from stdin.\n");
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(isnonnegint(line)){
				if((cluster = atoi(line)) == 0){
					EPRINT("Cluster number must be a positive integer.\n");
				}
			} else {
				EPRINT("Cluster number must be a positive integer.\n");
			}
			RESETSTR(line);
			size = 0;
			printf("Enter pin number: ");
			if((length = getline(&line,&size,stdin)) == -1){
				EPRINT("\nCould not read from stdin.\n");
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(isnonnegint(line)){
				pin = atoi(line);
			} else {
				EPRINT("Pin number must be a nonnegative integer.\n");
			}
			RESETSTR(line);
			size = 0;
		}

		//check that temp file does not already exist
		FILE *fprTemp = fopen("temp","r");
		if(fprTemp != NULL){
			fclose(fprTemp);
			EPRINT("File named \"temp\" already exists.\n");
		}

		//create temp file
		FILE *fpw = fopen("temp","w");
		if(fpw == NULL){
			EPRINT("Could not create file named \"temp\".\n");
		}

		//write to temp file
		int lineNumber;
		bool changed = false;
		line = NULL; //redundant
		size = 0; //redundant
		while(getline(&line,&size,fpr) != -1){
			lineNumber = strtol(line,NULL,10);
			if(lineNumber < number){
				fprintf(fpw,"%s",line);
			} else if(lineNumber == number){
				if(w){
					fprintf(fpw,"%d\t%d\t%d\n",number,cluster,pin);
				} //else do nothing
				changed = true;
			} else { //lineNumber > number
				if(!changed){
					if(w){
						fprintf(fpw,"%d\t%d\t%d\n",number,cluster,pin);
					} else { //d
						fclose(fpw);
						if(remove("temp") == -1){
							EPRINT("No such entry, and could not discard file named \"temp\".\n");
						}
						EPRINT("No such entry.\n");
					}
					changed = true;
				}
				fprintf(fpw,"%s",line);
			}
			RESETSTR(line);
			size = 0;
		}
		if(!changed){
			if(w){
				fprintf(fpw,"%d\t%d\t%d\n",number,cluster,pin);
			} else { //d
				fclose(fpw);
				if(remove("temp") == -1){
					EPRINT("No such entry, and could not discard file named \"temp\".\n");
				}
				EPRINT("No such entry.\n");
			}
			changed = true;
		}
		RESETSTR(line);
		size = 0;
		fclose(fpw);
	} else {
		EPRINT("Flag must be \"-r\", \"-w\", or \"-d\".\n");
	}

	//wrap up
	fclose(fpr);
	if(rename("temp",argv[2]) != 0){
		EPRINT("Could not replace topology file with file named \"temp\".\n");
	}

	return 0;
}

bool isnonnegint(char *string){
	for(int i = 0; string[i] != '\0'; i++){
		if(!isdigit(string[i])){
			return false;
		}
	}
	return true;
}
