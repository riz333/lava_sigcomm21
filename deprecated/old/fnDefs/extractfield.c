/* extractfield.c -- created by Will Sussman on June 20, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Takes in a 1-indexed column number and a row string
 * If extracting first column, beg = 0, loop through chars until first delimeter and set end
 * If extracting later column, count delimeters and set beg and end based on colNum
 * If a null is detected, error, return NULL
 * Otherwise create memory for extraction and return a null-terminated copy of [beg,end] */
char *extractfield(int colNum, char *row){
	if(colNum < 1){
		fprintf(stderr,"extractfield(): colNum must be 1 or greater\n");
		return NULL;
	}

	if(row == NULL){
		fprintf(stderr,"extractfield(): row is NULL\n");
		return NULL;
	}

	int beg, end;
	if(colNum == 1){
		beg = 0;
		for(int i = 0;; i++){
			if(row[i] == '\t' || row[i] == '\n'){
				end = i - 1;
				break;
			} else if(row[i] == '\0'){
				fprintf(stderr,"extractfield(): either missing delimiter or field does not exist\n");
				return NULL;
			}
		}
	} else { //colNum > 1
		int delimsReached = 0;
		for(int i = 0;; i++){
			if(row[i] == '\t' || row[i] == '\n'){
				if(++delimsReached == colNum - 1){ //preincrements delimsReached
					beg = i + 1;
				} else if(delimsReached == colNum){
					end = i - 1;
					break;
				}
			} else if(row[i] == '\0'){
				fprintf(stderr,"extractfield(): either missing delimiter or field does not exist\n");
				return NULL;
			}
		}
	} //beg and end are now set

	char *extraction = malloc(sizeof(*extraction) * (end - beg + 2));
	if(extraction == NULL){
		fprintf(stderr,"extractfield(): malloc() failed\n");
		return NULL;
	}
	strncpy(extraction,row + beg * sizeof(*extraction),sizeof(*extraction) * (end - beg + 1));
	extraction[end - beg + 1] = '\0';
	return extraction;
}
