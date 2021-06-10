/* hasduplicates.c -- created by Will Sussman on July 23, 2019 */

#include <stdbool.h>

/* Input list must be sorted and len must be nonnegative */
bool hasduplicates(int *path, int len){
	if(len == 0){
		return false;
	}

	for(int i = 1; i < len; i++){
		if(path[i] == path[i-1]){
			return true;
		}
	}
	return false;
}