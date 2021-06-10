/* bin2dec.c -- created by Will Sussman on July 24, 2019 */

#include <stdbool.h>
#include <math.h>

/* */
int bin2dec(bool *bin, int len){
	int dec = 0;
	for(int i = 0; i < len; i++){
		dec += bin[i] * pow(2,i);
	}
	return dec;
}