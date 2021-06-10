/* Created by Will Sussman on June 17, 2019
 * new_toputil is short for new topology utility
 * usage: new_toputil ((-a|-d) <filename> <eltID>)|((-r|-e) <filename> <eltID> <field>)|-i
 * options: new_toputil -a <filename> <eltID>
 *			new_toputil -d <filename> <eltID>
 *			new_toputil -r <filename> <eltID> <field>
 *			new_toputil -e <filename> <eltID> <field>
 *			new_toputil -i
 * <field> = all|cluster|pin|direction|prevElts|nextElts|sideElts
 * <direction> = N|NE|E|SE|S|SW|W|NW
 * gcc new_toputil.c -g3 -std=c99 -pedantic -Wall -o new_toputil */

#define _GNU_SOURCE
#define RESETSTR(string) {if(string != NULL){free(string); string = NULL;}}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void printUsage(void);
bool isnonnegint(char *string);
bool validDirection(char *string);
bool validList(char *string);

int main(int argc, char **argv){
	//check for too few or too many arguments
	if(argc < 2 || argc > 5){
		printUsage();
		exit(0);
	}

	//check flag and store it
	enum flags {add,delete,read,edit,info} flag;
	if(!strcmp(argv[1],"-a")){
		flag = add;
	} else if(!strcmp(argv[1],"-d")){
		flag = delete;
	} else if(!strcmp(argv[1],"-r")){
		flag = read;
	} else if(!strcmp(argv[1],"-e")){
		flag = edit;
	} else if(!strcmp(argv[1],"-i")){
		flag = info;
	} else {
		fprintf(stderr,"Invalid flag.\n");
		exit(0);
	}

	//check for too few or too many arguments by flag
	if(((flag == add || flag == delete) && argc != 4) ||
		((flag == read || flag == edit) && argc != 5) ||
		(flag == info && argc != 2)){
		printUsage();
		exit(0);
	}

	//check eltID as applicable and store it
	long int eltID;
	if(flag != info){
		if(!isnonnegint(argv[3]) || (eltID = atol(argv[3])) == 0L){ //eltID is now set
			fprintf(stderr,"<eltID> must be a positive integer.\n");
			exit(0);
		}
	}

	//check field as applicable and store it
	enum fields {all,clusterCol,pinCol,directionCol,prevEltsCol,nextEltsCol,sideEltsCol} field;
	if(flag == read || flag == edit){
		if(!strcmp(argv[4],"all")){
			field = all;
		} else if(!strcmp(argv[4],"cluster")){
			field = clusterCol;
		} else if(!strcmp(argv[4],"pin")){
			field = pinCol;
		} else if(!strcmp(argv[4],"direction")){
			field = directionCol;
		} else if(!strcmp(argv[4],"prevElts")){
			field = prevEltsCol;
		} else if(!strcmp(argv[4],"nextElts")){
			field = nextEltsCol;
		} else if(!strcmp(argv[4],"sideElts")){
			field = sideEltsCol;
		} else {
			fprintf(stderr,"Invalid <field>.\n");
			exit(0);
		}
	}

	//fork based on flag
	if(flag == info){
		printf("Information to be added.\n");
		return 0;
	}

	//all other flags require preliminary entry check
	FILE *topfile = fopen(argv[2],"r");
	if(topfile == NULL){
		fprintf(stderr,"Could not open file named \"%s\".\n",argv[2]);
		exit(0);
	}
	long int lineNum;
	ssize_t length;
	bool found = false;
	long int cluster, pin;
	char *direction = NULL, *prevElts = NULL, *nextElts = NULL, *sideElts = NULL;
	char *line = NULL;
	size_t size = 0;
	while(getline(&line,&size,topfile) != -1){ //traverse topfile
		if((lineNum = strtol(line,NULL,10)) == eltID){ //entry exists; lineNum is now set
			found = true;
			if(flag == add){
				fprintf(stderr,"Entry for eltID %ld already exists.\n",eltID);
				exit(0);
			} else {
				int delimsReached = 0, beg, end;
				char *addr;
				for(int i = 0; line[i] != '\0'; i++){
					if(line[i] == '\t' || line[i] == '\n'){
						switch(++delimsReached){
							case 1:{ //reached cluster
								cluster = strtol(line + i + 1,&addr,10);
								i = addr - line - 1;
								break;
							}
							case 2:{ //reached pin
								pin = strtol(line + i + 1,&addr,10);
								i = addr - line - 1;
								break;
							}
							case 3:{ //reached direction
								beg = i + 1;
								break;
							}
							case 4:{ //reached prevElts
								end = i - 1;
								direction = malloc(sizeof(*direction) * (end - beg + 2));
								if(direction == NULL){
									fprintf(stderr,"Could not allocate memory for the direction.\n");
									exit(0);
								}
								strncpy(direction,line + beg,sizeof(*direction) * (end - beg + 1));
								direction[end - beg + 1] = '\0';
								beg = i + 1;
								break;
							}
							case 5:{ //reached nextElts
								end = i - 1;
								prevElts = malloc(sizeof(*prevElts) * (end - beg + 2));
								if(prevElts == NULL){
									fprintf(stderr,"Could not allocate memory for the list of previous elements.\n");
									exit(0);
								}
								strncpy(prevElts,line + beg,sizeof(*prevElts) * (end - beg + 1));
								prevElts[end - beg + 1] = '\0';
								beg = i + 1;
								break;
							}
							case 6:{ //reached sideElts
								end = i - 1;
								nextElts = malloc(sizeof(*nextElts) * (end - beg + 2));
								if(nextElts == NULL){
									fprintf(stderr,"Could not allocate memory for the list of next elements.\n");
									exit(0);
								}
								strncpy(nextElts,line + beg,sizeof(*nextElts) * (end - beg + 1));
								nextElts[end - beg + 1] = '\0';
								beg = i + 1;
								break;
							}
							case 7:{ //reached end
								end = i - 1;
								sideElts = malloc(sizeof(*sideElts) * (end - beg + 2));
								if(sideElts == NULL){
									fprintf(stderr,"Could not allocate memory for the list of side elements.\n");
									exit(0);
								}
								strncpy(sideElts,line + beg,sizeof(*sideElts) * (end - beg + 1));
								sideElts[end - beg + 1] = '\0';
								break;
							}
						}
					}
				}
			}
		} else if(lineNum > eltID){ //entry does not exist
			break; //the while loop
		}
		RESETSTR(line);
		size = 0;
	} //endwhile
	if(!found){ //entry does not exist
		if(flag == delete || flag == read || flag == edit){
			fprintf(stderr,"Entry for eltID %ld does not exist.\n",eltID);
			exit(0);
		}
	}
	RESETSTR(line);
	size = 0;

	if(flag == read){
		switch(field){
			case all:{
				printf("all:\t%ld\t%ld\t%ld\t%s\t%s\t%s\t%s\n",eltID,cluster,pin,direction,prevElts,nextElts,sideElts);
				break;
			}
			case clusterCol:{
				printf("cluster: %ld\n",cluster);
				break;
			}
			case pinCol:{
				printf("pin: %ld\n",pin);
				break;
			}
			case directionCol:{
				printf("direction: %s\n",direction);
				break;
			}
			case prevEltsCol:{
				printf("prevElts: %s\n",prevElts);
				break;
			}
			case nextEltsCol:{
				printf("nextElts: %s\n",nextElts);
				break;
			}
			case sideEltsCol:{
				printf("sideElts: %s\n",sideElts);
				break;
			}
		}
		return 0;
	}

	/* replace topfile with altered version */
	//check that temp file does not already exist
	FILE *tempfile = fopen("temp","r");
	if(tempfile != NULL){
		fprintf(stderr,"File named \"temp\" already exists.\n");
		exit(0);
	}

	//create temp file
	tempfile = fopen("temp","w");
	if(tempfile == NULL){
		fprintf(stderr,"Could not create file named \"temp\".\n");
		exit(0);
	}

	//write to temp file
	rewind(topfile);
	if(flag == delete){
		RESETSTR(line);
		size = 0;
		while(getline(&line,&size,topfile) != -1){ //traverse topfile
			if(strtol(line,NULL,10) != eltID){
				fprintf(tempfile,"%s",line); //copy over
			}
			RESETSTR(line);
			size = 0;
		}
		RESETSTR(line);
		size = 0;
		printf("Deleted entry for eltID %ld.\n",eltID);
	} else if(flag == add || flag == edit){
		if(flag == add || (flag == edit && (field == all || field == clusterCol))){
			if(flag == edit){
				printf("\nCluster number about to be overwritten: %ld\n",cluster);
			}
			printf("Enter the cluster number: ");
			RESETSTR(line);
			size = 0;
			if((length = getline(&line,&size,stdin)) == -1){
				fprintf(stderr,"\nCould not read from stdin.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(!isnonnegint(line) || (cluster = atol(line)) == 0L){ //cluster is now set
				fprintf(stderr,"Cluster number must be a positive integer.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
		}

		if(flag == add || (flag == edit && (field == all || field == pinCol))){
			if(flag == edit){
				printf("\nPin number about to be overwritten: %ld\n",pin);
			}
			printf("Enter the pin number: ");
			RESETSTR(line);
			size = 0;
			if((length = getline(&line,&size,stdin)) == -1){
				fprintf(stderr,"\nCould not read the pin number from stdin.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(!isnonnegint(line)){
				fprintf(stderr,"Pin number must be a nonnegative integer.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			pin = atol(line);
		}

		if(flag == add || (flag == edit && (field == all || field == directionCol))){
			if(flag == edit){
				printf("\nDirection about to be overwritten: %s\n",direction);
				RESETSTR(direction);
			}
			printf("Enter the direction: ");
			RESETSTR(line);
			size = 0;
			if((length = getline(&line,&size,stdin)) == -1){
				fprintf(stderr,"\nCould not read the direction from stdin.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(!validDirection(line)){
				fprintf(stderr,"Invalid <direction>.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			direction = malloc(sizeof(*direction) * length);
			if(direction == NULL){
				fprintf(stderr,"Could not allocate memory for the direction.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			strcpy(direction,line);
		}

		if(flag == add || (flag == edit && (field == all || field == prevEltsCol))){
			if(flag == edit){
				printf("\nList of previous elements about to be overwritten: %s\n",prevElts);
				RESETSTR(prevElts);
			}
			printf("Enter the previous elements separated by commas (or - if none): ");
			RESETSTR(line);
			size = 0;
			if((length = getline(&line,&size,stdin)) == -1){
				fprintf(stderr,"\nCould not read the list of previous elements from stdin.\n");
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(!validList(line)){
				fprintf(stderr,"Invalid list of previous elements.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			prevElts = malloc(sizeof(*prevElts) * length);
			if(prevElts == NULL){
				fprintf(stderr,"Could not allocate memory for the list of previous elements.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			strcpy(prevElts,line);
		}

		if(flag == add || (flag == edit && (field == all || field == nextEltsCol))){
			if(flag == edit){
				printf("\nList of next elements about to be overwritten: %s\n",nextElts);
				RESETSTR(nextElts);
			}
			printf("Enter the next elements separated by commas (or - if none): ");
			RESETSTR(line);
			size = 0;
			if((length = getline(&line,&size,stdin)) == -1){
				fprintf(stderr,"\nCould not read the list of next elements from stdin.\n");
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(!validList(line)){
				fprintf(stderr,"Invalid list of next elements.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			nextElts = malloc(sizeof(*nextElts) * length);
			if(nextElts == NULL){
				fprintf(stderr,"Could not allocate memory for the list of next elements.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			strcpy(nextElts,line);
		}

		if(flag == add || (flag == edit && (field == all || field == sideEltsCol))){
			if(flag == edit){
				printf("\nList of side elements about to be overwritten: %s\n",sideElts);
				RESETSTR(sideElts);
			}
			printf("Enter the side elements separated by commas (or - if none): ");
			RESETSTR(line);
			size = 0;
			if((length = getline(&line,&size,stdin)) == -1){
				fprintf(stderr,"\nCould not read the list of side elements from stdin.\n");
			}
			line[length - 1] = '\0'; //overwrite '\n'
			if(!validList(line)){
				fprintf(stderr,"Invalid list of side elements.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			sideElts = malloc(sizeof(*sideElts) * length);
			if(sideElts == NULL){
				fprintf(stderr,"Could not allocate memory for the list of side elements.\n");
				fclose(tempfile);
				remove("temp");
				exit(0);
			}
			strcpy(sideElts,line);
		}

		bool printed = false;
		RESETSTR(line);
		size = 0;
		while(getline(&line,&size,topfile) != -1){ //traverse topfile
			if(strtol(line,NULL,10) == eltID){ //must be editing
				fprintf(tempfile,"%ld\t%ld\t%ld\t%s\t%s\t%s\t%s\n",eltID,cluster,pin,direction,prevElts,nextElts,sideElts);
				printed = true;
				RESETSTR(line);
				size = 0;
				continue;
			}
			if(!printed && strtol(line,NULL,10) > eltID){ //must be adding
				fprintf(tempfile,"%ld\t%ld\t%ld\t%s\t%s\t%s\t%s\n",eltID,cluster,pin,direction,prevElts,nextElts,sideElts);
				printed = true;
			}
			fprintf(tempfile,"%s",line); //copy over
			RESETSTR(line);
			size = 0;
		}
		if(!printed){ //must be adding
			fprintf(tempfile,"%ld\t%ld\t%ld\t%s\t%s\t%s\t%s\n",eltID,cluster,pin,direction,prevElts,nextElts,sideElts);
		}
		RESETSTR(line);
		size = 0;
	}

	fclose(topfile);
	fclose(tempfile);
	if(rename("temp",argv[2]) != 0){
		fprintf(stderr,"Could not replace topology file with file named \"temp\".\n");
	}
	return 0;
}

void printUsage(void){
 	fprintf(stderr,"options: new_toputil -a <filename> <eltID>\n");
	fprintf(stderr,"         new_toputil -d <filename> <eltID>\n");
	fprintf(stderr,"         new_toputil -r <filename> <eltID> <field>\n");
	fprintf(stderr,"         new_toputil -e <filename> <eltID> <field>\n");
	fprintf(stderr,"         new_toputil -i\n");
	fprintf(stderr,"<field> = all|cluster|pin|direction|prevElts|nextElts|sideElts\n");
	fprintf(stderr,"<direction> = N|NE|E|SE|S|SW|W|NW\n");
	return;
}

bool isnonnegint(char *string){
	for(int i = 0; string[i] != '\0'; i++){
		if(!isdigit(string[i])){
			return false;
		}
	}
	return true;
}

bool validDirection(char *string){
	if(!strcmp(string,"NW") ||
		!strcmp(string,"N") ||
		!strcmp(string,"NE") ||
		!strcmp(string,"E") ||
		!strcmp(string,"SE") ||
		!strcmp(string,"S") ||
		!strcmp(string,"SW") ||
		!strcmp(string,"W")){
		return true;
	}
	return false;
}

bool validList(char *string){
	if(!strcmp(string,"-")){
		return true;
	}
	for(int i = 0; string[i] != '\0'; i++){
		if(!isdigit(string[i]) && string[i] != ','){
			return false;
		}
	}
	return true;
}
