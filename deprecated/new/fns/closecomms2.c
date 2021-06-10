// closecomms2.c -- created by Will Sussman on December 30, 2019

#include <unistd.h>

#include "/home/riz3/willsLibrary/new/libwill.h"

//	closecomms2(sock);
int closecomms2(int sock){
	// struct List *currnode = list;
	// while(currnode != NULL){
		if(close(sock) == -1){
			fprintf(stderr,"closecomms2() failed\n");
			return 1;
		}
		// currnode = currnode->next;
	// }

	return 0;

	// for(int i = 1; i < numgroups; i++){ //USED TO START FROM 0 FOR GENERALITY
	// 	if(close(socks[i]) == -1){
	// 		fprintf(stderr,"closecomms(): WARNING: close() failed for socket %d\n",socks[i]);
	// 		// return false;
	// 	} else {
	// 		printf("closecomms(): close() succeeded for socket %d\n",socks[i]);
	// 	}
	// }
	// return true;
}
