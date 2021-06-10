/* genfan.c -- created by Will Sussman on June 21, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "liblawa.h"

/* Recursive function
 * Takes in an input file pointer, a root ID number, a field number,
 *     an output file pointer, an int array and its size, and a partial paths setting
 * currpath and pathlen should initially be NULL and 0, respectively
 * The input and output files should have already been opened for reading and writing, respectively
 * Adds the root ID number to the int array and increments the size var
 * Looks up the line for the root, extracts the field from the line, and parses the field
 * If the partial paths setting is on, the output file will include partial paths
 * Otherwise the output file will only include full paths by printing in the base case
 * The func is called recursively on each of the IDs parsed from the field extracted from the line */
bool genfan(FILE *infile, int rootID, int field, FILE *outfile, int *currpath, int pathlen, bool partialPaths){
	if(pathlen == 0){
		if(infile == NULL){
			fprintf(stderr,"genfan(): infile is NULL\n");
			return false;
		}
		if(outfile == NULL){
			fprintf(stderr,"genfan(): outfile is NULL\n");
			return false;
		}
		if(currpath != NULL){
			fprintf(stderr,"genfan(): WARNING: currpath is not initially NULL\n");
			//don't return
		}
	}

	currpath = realloc(currpath,sizeof(*currpath) * ++pathlen); //preincrements pathlen
	if(currpath == NULL){
		fprintf(stderr,"genfan(): beginning realloc() failed\n");
		return false;
	}
	currpath[pathlen - 1] = rootID;

	char *line = getlineid(infile,rootID);
	if(line == NULL){
		fprintf(stderr,"genfan(): could not get line %d\n",rootID);
		return false;
	}
	char *strlist = extractfield(field,line);
	if(strlist == NULL){
		fprintf(stderr,"genfan(): could not extract field %d\n",field);
		return false;
	}
	resetptr(line);
	int *intlist, numInts;
	if(!parselist(strlist,&intlist,&numInts)){
		fprintf(stderr,"genfan(): could not extract field %d\n",field);
		return false;
	}
	resetptr(strlist);
	if(partialPaths){
		fprintints(stdout,currpath,pathlen);
		fprintints(outfile,currpath,pathlen);
	} else if(numInts == 0){ //just full paths
		fprintints(stdout,currpath,pathlen);
		fprintints(outfile,currpath,pathlen);
	}
	for(int i = 0; i < numInts; i++){
		genfan(infile,intlist[i],field,outfile,currpath,pathlen,partialPaths);
	}
	resetptr(intlist);
	currpath = realloc(currpath,sizeof(*currpath) * --pathlen); //predecrements pathlen
	if(currpath == NULL && pathlen != 0){
		fprintf(stderr,"genfan(): end realloc() failed\n");
		return false;
	}
	return true;
}
