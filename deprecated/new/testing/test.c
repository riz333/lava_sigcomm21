// test.c -- created by Will Sussman on November 22, 2019

#include <stdio.h>
#include <stdlib.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int main(int argc, char **argv){
	
	/* declare a file pointer */
	FILE    *infile;
	char    *buffer;
	long    numbytes;
	 
	/* open an existing file for reading */
	infile = fopen("testtable", "r");
	 
	/* quit if the file does not exist */
	if(infile == NULL)
	    return 1;
	 
	/* Get the number of bytes */
	fseek(infile, 0L, SEEK_END);
	numbytes = ftell(infile);
	 
	/* reset the file position indicator to 
	the beginning of the file */
	fseek(infile, 0L, SEEK_SET);	
	 
	/* grab sufficient memory for the 
	buffer to hold the text */
	buffer = (char*)calloc(numbytes, sizeof(char));	
	 
	/* memory error */
	if(buffer == NULL)
	    return 1;
	 
	/* copy all the text into the buffer */
	fread(buffer, sizeof(char), numbytes, infile);
	fclose(infile);
	 
	/* confirm we have read the file by
	outputing it to the console */
	// printf("The file called test.dat contains this text\n\n%s", buffer);
	 
	/* free the memory we used for the buffer */
	// free(buffer);

	printf("%s\n",makesubstr(movetocell(atoi(argv[1]),atoi(argv[2]),buffer),0,2));
	free(buffer);
}