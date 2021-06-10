/* fprintints.c -- created by Will Sussman on June 21, 2019 */

#include <stdio.h>
#include <stdbool.h>

/* Takes in a file pointer, an int array, and the size of the int array
 * The file should have already been opened for writing
 * Prints the int array to the file with format "%d,%d,...,%d\n" (or "-\n" if none) */
bool fprintints(FILE *outfile, int *ints, int numints){
	if(outfile == NULL){
		fprintf(stderr,"fprintints(): outfile is NULL\n");
		return false;
	}

	if(numints == 0){
		fprintf(outfile,"-\n");
		return true;
	}

	for(int i = 0; i < numints - 1; i++){
		fprintf(outfile,"%d,",ints[i]);
	}
	fprintf(outfile,"%d\n",ints[numints - 1]);
	return true;
}