/* resetptr.c -- created by Will Sussman on June 20, 2019 */

 #include <stdlib.h>

/* Takes in a pointer
 * If the pointer is not NULL, it is freed and set to NULL
 * Otherwise nothing happens */
void resetptr(void *pointer){
	if(pointer != NULL){
		free(pointer);
		pointer = NULL;
	}
	return;
}
