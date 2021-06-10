/* closecomms.c -- created by Will Sussman on July 24, 2019 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

bool closecomms(int *socks, int numgroups){
	for(int i = 1; i < numgroups; i++){ //USED TO START FROM 0 FOR GENERALITY
		if(close(socks[i]) == -1){
			fprintf(stderr,"closecomms(): WARNING: close() failed for socket %d\n",socks[i]);
			// return false;
		} else {
			printf("closecomms(): close() succeeded for socket %d\n",socks[i]);
		}
	}
	return true;
}
