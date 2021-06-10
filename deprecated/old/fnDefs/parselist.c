/* parselist.c -- created by Will Sussman on June 21, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


/* Takes in a string with a list of numbers and pointer to an int array and its size var
 * If the string is -, int array is set to NULL, size var is set to 0, and func returns true
 * Otherwise scans string until first null char and counts commas
 * Then sets size var, creates memory for int array, fills the int array, and returns true
 * Note to self: I use a very cool trick to do the conversion! */
bool parselist(char *strlist, int **intlist, int *numInts){
	if(strlist == NULL){
		fprintf(stderr,"parselist(): strlist is NULL\n");
		return false;
	}
	if(intlist == NULL){
		fprintf(stderr,"parselist(): intlist is NULL\n");
		return false;
	}
	if(*intlist != NULL){
		fprintf(stderr,"parselist(): WARNING: *intlist is not NULL, memory may leak\n");
		//don't return
	}
	if(numInts == NULL){
		fprintf(stderr,"parselist(): numInts is NULL\n");
		return false;
	}

	if(!strcmp(strlist,"-")){
		*intlist = NULL;
		*numInts = 0;
		return true;
	}
	int numCommas = 0;
	for(int i = 0; strlist[i] != '\0'; i++){
		if(strlist[i] == ','){
			numCommas++;
		}
	}
	*numInts = numCommas + 1;
	*intlist = malloc(sizeof(**intlist) * *numInts);
	if(*intlist == NULL){
		fprintf(stderr,"parselist(): malloc() failed\n");
		return false;
	}
	char *ptr = strlist - sizeof(*strlist); //I love this!
	for(int i = 0; i < *numInts; i++){
		(*intlist)[i] = (int)strtol(ptr + sizeof(*ptr),&ptr,10);
	}
	return true;
}