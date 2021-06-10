// min_fencing.c -- created by Will Sussman on January 30, 2020

#include <stdio.h>
#include <stdlib.h>

#define BUFFSIZE 32

void print(int *A, int sizeA);

int main(int argc, char **argv){

	if(argc < 2){
		fprintf(stderr,"usage: min_fencing L d[0] d[1] ... d[n-1]\n");
		exit(1);
	}

	float L = atof(argv[1]);
	printf("L is %f\n",L);
	int sizeD = argc - 2;
	printf("sizeD is %d\n",sizeD);
	int n = sizeD - 2;
	printf("n is %d\n",n);
	float *d = malloc(sizeof(*d) * sizeD);
	for(int j = 0; j < sizeD; j++){
		d[j] = atof(argv[j+2]);
		printf("d[%d] is %f\n",j,d[j]);
	}

	int i = 1;
	printf("i initialized to 0\n");
	int k = 1;
	printf("k initialized to 1\n");
	int A[BUFFSIZE];
	A[0] = 0;
	printf("A[0] initialized to 0\n");
	int sizeA = 1;
	printf("sizeA initialized to 1\n");

	float diff;

	print(A,sizeA);

	while(i < n + 2){
		printf("\nNew iteration\n");

		diff = d[i] - d[A[k-1]];
		printf("i is %d, d[i] is %f, k is %d, A[k-1] is %d, d[A[k-1]] is %f, so diff is %f, sizeA is %d\n",i,d[i],k,A[k-1],d[A[k-1]],diff,sizeA);

		if(diff <= L){
			printf("diff <= L\n");
			if(sizeA != 1){
				sizeA--;
				printf("decrementing sizeA to %d\n",sizeA);
				print(A,sizeA);
			}

		} else {
			printf("else: diff > L\n");
			printf("k is %d, incrementing to %d\n",k,k+1);
			k++;
		}
		printf("sizeA is %d, setting A[sizeA] to %d, incrementing sizeA to %d\n",sizeA,i,sizeA+1);
		A[sizeA++] = i;
		print(A,sizeA);
		i++;
		printf("incrementing i to %d\n",i);
	}
	printf("After loop\n");
	printf("i is %d, k is %d, sizeA is %d\n",i,k,sizeA);

	print(A,sizeA);
	// printf("after end: %d\n",A[sizeA]);

	return 0;

}

void print(int *A, int sizeA){
	printf("A: [");
	for(int j = 0; j < sizeA; j++){
		printf(" %d ",A[j]);
	}
	printf("]\n");
	return;
}
