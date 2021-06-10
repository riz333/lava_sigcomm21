/* pathcpy.c -- created by Will Sussman on July 22, 2019 */

#include <stdio.h>
#include <stdlib.h>

/* */
int *pathcpy(int *path, int len){
	 int *copy = malloc(sizeof(*copy) * len);
	 if(copy == NULL){
	 	fprintf(stderr,"pathcpy(): malloc() failed\n");
	 	return NULL;
	 }
	 for(int i = 0; i < len; i++){
	 	copy[i] = path[i];
	 }
	 return copy;
}