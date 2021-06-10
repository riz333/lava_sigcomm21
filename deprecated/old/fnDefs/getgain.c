/* getgain.c -- created by Will Sussman on July 9, 2019 */

#include <stdio.h>
#include <stdlib.h>

/* */
float getgain(){
	float gain = 25.0 * rand()/(float)RAND_MAX;
	printf("getgain(): gain is %f\n",gain);
	return gain;
}