/* willsAlgorithm.c -- created by Will Sussman on July 9, 2019 */

#define _GNU_SOURCE
#define PORT 4012

#include <stdio.h>
#include <stdlib.h>
#include "/home/riz3/willsLibrary/libfiles/liblawa.h"

int main(int argc, char **argv){

	if(argc != 5){
		fprintf(stderr,"usage: willsAlgorithm topfile fullpathsfile gainthresh maxtries\n");
		exit(0);
	}

	FILE *topfile = fopen(argv[1],"r");
	if(topfile == NULL){
		fprintf(stderr,"main(): could not open file named \"%s\"\n",argv[1]);
		exit(0);
	}

	FILE *fullpathsfile = fopen(argv[2],"r");
	if(fullpathsfile == NULL){
		fprintf(stderr,"main(): could not open file named \"%s\"\n",argv[2]);
		exit(0);
	}

	float gainthresh = strtof(argv[3],NULL);
	int maxtries = atoi(argv[4]);
	if(maxtries < 0){
		fprintf(stderr,"main(): maxtries must be a nonnegative integer\n");
		exit(0);
	}

	/* In the interest of simplicity, I leave holes
	 * in the arrays at the expense of some memory. */
	int numgroups = 11;
	int maxpins = 5;

	char *line = NULL;
	size_t size = 0;
	int *pathIDs = NULL;
	int pathlen = 0;
	int *currpath = NULL;
	int currlen = 0;
	int endCount = 0;
	float gain;
	bool foundGoodPath = false;
	float bestGainSoFar;
	int *bestPathSoFar = NULL;
	int bestPathLen = 0;
	bool **stateTable = malloc(sizeof(*stateTable) * numgroups);
	if(stateTable == NULL){
		fprintf(stderr,"main(): malloc() failed\n");
		exit(0);
	}
	for(int i = 0; i < numgroups; i++){
		stateTable[i] = malloc(sizeof(*(stateTable[i])) * maxpins);
		if(stateTable[i] == NULL){
			fprintf(stderr,"main(): malloc() failed\n");
			exit(0);
		}
		for(int j = 0; j < maxpins; j++){
			stateTable[i][j] = 0;
		}
	}
	bool *groupChanges = malloc(sizeof(*groupChanges) * numgroups);
	if(groupChanges == NULL){
		fprintf(stderr,"changepath(): malloc() failed\n");
		return false;
	}
	for(int i = 0; i < numgroups; i++){
		groupChanges[i] = 0;
	}
	int *socks = opencomms(numgroups,PORT);	
	if(socks == NULL){
		fprintf(stderr,"main(): opencomms() failed\n");
		exit(0);
	}
	while(1){
		if(getline(&line,&size,fullpathsfile) == -1){ //EOF; sets line
			resetptr(line);
			size = 0;
			if(foundGoodPath){
				resetptr(currpath);
				currlen = 0;
				break;
			}
			if(++endCount == maxtries){ //preincrements endCount
				fprintf(stderr,"main(): reached maximum number of tries\n");
				exit(0);
			}
			rewind(fullpathsfile);
			continue;
		}
		if(!parselist(line,&pathIDs,&pathlen)){ //sets path vars
			fprintf(stderr,"main(): parselist() failed\n");
			exit(0);
		}
		resetptr(line);
		size = 0;
		if(!changepath(topfile,currpath,currlen,pathIDs,pathlen,stateTable,groupChanges,numgroups,maxpins,socks)){
			fprintf(stderr,"main(): changepath() failed\n");
			exit(0);
		}
		if(!updatepathvars(&currpath,&currlen,&pathIDs,&pathlen)){ //clears path vars
			fprintf(stderr,"main(): updatepathvars() failed\n");
			exit(0);
		}
		printf("\nCurrent path is ");
		fprintints(stdout,currpath,currlen);

		gain = getgain();
		if(!foundGoodPath && gain > gainthresh){ //found first contender
			printf("Found first contender\n");
			foundGoodPath = true;
			bestGainSoFar = gain;
			bestPathSoFar = pathcpy(currpath,currlen);
			bestPathLen = currlen;
		}
		if(foundGoodPath && gain > bestGainSoFar){ //found another contender
			printf("Found another contender\n");
			bestGainSoFar = gain;
			resetptr(bestPathSoFar);
			bestPathSoFar = pathcpy(currpath,currlen);
			bestPathLen = currlen;
		}
	}
	//bestPathSoFar and bestPathLen still set
	printf("\nBest path found is ");
	fprintints(stdout,bestPathSoFar,bestPathLen);
	printf("Best gain found is %f\n",bestGainSoFar);

	if(!closecomms(socks,numgroups)){
		fprintf(stderr,"main(): closecomms() failed\n");
		exit(0);
	}

	resetptr(groupChanges);
	for(int i = 0; i < numgroups; i++){
		resetptr(stateTable[i]);
	}
	resetptr(stateTable);

	resetptr(bestPathSoFar);
	bestPathLen = 0;

	fclose(topfile);
	topfile = NULL;
	fclose(fullpathsfile);
	fullpathsfile = NULL;
	return 0;
}
