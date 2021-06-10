/* updatepathvars.c -- created by Will Sussman on July 22, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "liblawa.h"

/* Takes in current path variables and desired path variables
 * Frees the current path array, sets the current path vars to desired path vars,
 * and clears the desired path vars
 * Returns false if input var is NULL, otherwise ultimately returns true */
bool updatepathvars(int **currpath, int *currlen, int **pathIDs, int *pathlen){
	if(currpath == NULL){
		fprintf(stderr,"updatepathvars(): currpath is NULL\n");
		return false;
	}
	if(currlen == NULL){
		fprintf(stderr,"updatepathvars(): currlen is NULL\n");
		return false;
	}
	if(pathIDs == NULL){
		fprintf(stderr,"updatepathvars(): pathIDs is NULL\n");
		return false;
	}
	if(pathlen == NULL){
		fprintf(stderr,"updatepathvars(): pathlen is NULL\n");
		return false;
	}

	resetptr(*currpath);
	*currpath = *pathIDs;
	*pathIDs = NULL; //but don't free it (would corrupt *currpath)
	*currlen = *pathlen;
	*pathlen = 0;

	return true;
}