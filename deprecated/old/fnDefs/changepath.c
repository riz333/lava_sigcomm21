/* changepath.c -- created by Will Sussman on July 9, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <math.h>
#include "liblawa.h"

/* */
bool changepath(FILE *topfile, int *currpath, int currlen, int *pathIDs, int pathlen, bool **stateTable, bool *groupChanges, int numgroups, int maxpins, int *socks){
	if(topfile == NULL){
		fprintf(stderr,"changepath(): topfile is NULL\n");
		return false;
	}
	if(stateTable == NULL){
		fprintf(stderr,"changepath(): stateTable is NULL\n");
		return false;
	}
	if(groupChanges == NULL){
		fprintf(stderr,"changepath(): groupChanges is NULL\n");
		return false;
	}

	//sort and check for duplicates
	qsort(currpath,currlen,sizeof(*currpath),intcmp);
	qsort(pathIDs,pathlen,sizeof(*pathIDs),intcmp);
	if(hasduplicates(currpath,currlen)){
		fprintf(stderr,"changepath(): currpath has duplicates\n");
		return false;
	}
	if(hasduplicates(pathIDs,pathlen)){
		fprintf(stderr,"changepath(): pathIDs has duplicates\n");
		return false;
	}

	int i = 0;
	int j = 0;
	while(i < currlen){
		if(j < pathlen){
			if(currpath[i] == pathIDs[j]){
				//leave it alone
				i++;
				j++;
			} else if(currpath[i] < pathIDs[j]){
				//turn off currpath[i]
				if(!changestate(currpath[i],0,topfile,stateTable,groupChanges,numgroups,maxpins)){
					fprintf(stderr,"changepath(): changestate() failed\n");
					return false;
				}
				i++;
			} else { //currpath[i] > pathIDs[j]
				//turn on pathIDs[j]
				if(!changestate(pathIDs[j],1,topfile,stateTable,groupChanges,numgroups,maxpins)){
					fprintf(stderr,"changepath(): changestate() failed\n");
					return false;
				}
				j++;
			}
		} else { //j >= pathlen
			//turn off currpath[i]
			if(!changestate(currpath[i],0,topfile,stateTable,groupChanges,numgroups,maxpins)){
				fprintf(stderr,"changepath(): changestate() failed\n");
				return false;
			}
			i++;
		}
	}
	while(j < pathlen){
		//turn on pathIDs[j]
		if(!changestate(pathIDs[j],1,topfile,stateTable,groupChanges,numgroups,maxpins)){
			fprintf(stderr,"changepath(): changestate() failed\n");
			return false;
		}
		j++;
	}

	int dec;
	int len;
	char *msg;
	for(int i = 0; i < numgroups; i++){
		dec = bin2dec(stateTable[i],maxpins);
		if(dec == 0){
			len = 2;
		} else { //dec should be a positive integer
			len = floor(log10(dec)) + 2;
		}
		msg = malloc(sizeof(*msg) * len);
		if(msg == NULL){
			fprintf(stderr,"changepath(): malloc() failed\n");
			return false;
		}
		sprintf(msg,"%d",dec);
		// if(send(socks[i],msg,len,0) == -1){
		// 	fprintf(stderr,"changepath(): send() failed\n");
		// 	return false;
		// }
	}
	return true;
}
