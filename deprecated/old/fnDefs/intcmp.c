/* intcmp.c -- created by Will Sussman on July 23, 2019 */

int intcmp(const void *arg1ptr, const void *arg2ptr){
	int arg1 = *(int *)arg1ptr;
	int arg2 = *(int *)arg2ptr;
	return arg1 - arg2; //all that matters is the sign
}