// numdigits.c -- created by Will Sussman on December 20, 2019

int numdigits(long val){
	int count = 0;
	while(val != 0){
		val /= 10;
		count++;
	}
	return count;
}