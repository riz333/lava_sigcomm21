/* alldigits.c -- created by Will Sussman on June 20, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/* Takes in a string to scan
 * Loops through chars in string until first null
 * Returns false if any scanned char is not a digit
 * Otherwise returns true */
bool alldigits(char *string){
	if(string == NULL){
		fprintf(stderr,"alldigits(): string is NULL\n");
		return false;
	}

	for(int i = 0; string[i] != '\0'; i++){
		if(!isdigit(string[i])){
			return false;
		}
	}
	return true;
}
