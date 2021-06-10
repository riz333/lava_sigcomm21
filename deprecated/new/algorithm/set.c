// set.c -- created by Will Sussman on December 30, 2019

#define PORT 4012
#define TIMEOUT 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int main(int argc, char **argv){
	// if(argc != 6){
	// 	fprintf(stderr,"usage: alg topfile startid startdir endid enddir\n");
	// 	return 1;
	// }

	if(argc != 3){
		fprintf(stderr,"usage: set ipaddr ccn\n");
		return 1;
	}

	if(strlen(argv[2]) < 3){
		fprintf(stderr,"usage: set ipaddr ccn\n");
		return 1;
	}

	char *ipaddr = argv[1];
	char dir1 = argv[2][0];
	char dir2 = argv[2][1];
	char phase = argv[2][2];

	int sock;
	fprintf(stdout,"About to open comms\n");
	if(opencomms2(ipaddr,PORT,TIMEOUT,&sock)){
		fprintf(stderr,"opencomms2() failed, returning early\n");
		return 1;
	}
	fprintf(stdout,"Done opening comms\n");

	// int numpaths = 0;
	// struct Path path;
	// path.len = 0;
	// path.elts = NULL;
	// struct Path *pathset = NULL;
	// // printf("about to genpaths2()\n");
	// genpaths2(&path,list,&pathset,startid,endid,&numpaths);
	// if(numpaths == 0){
	// 	fprintf(stderr,"No path exists\n");
	// 	return 1;
	// }

	// int minpath = 0;
	// for(int i = 1; i < numpaths; i++){
	// 	if(pathset[i].len < pathset[minpath].len){
	// 		minpath = i;
	// 	}
	// }

	// struct Buffer bytes;
	// // printf("about to prepbytes()\n");
	// prepbytes(pathset[minpath],&bytes,list);
	// if(startdir == bytes.contents[0]){
	// 	fprintf(stderr,"Invalid startdir\n");
	// 	return 1;
	// }
	// if(enddir == bytes.contents[bytes.size - 1]){
	// 	fprintf(stderr,"Invalid enddir\n");
	// 	return 1;
	// }
	// for(int i = 0; i < bytes.size; i++){
	// 	printf("%c",bytes.contents[i]);
	// }
	// printf("about to sendcmd()\n");

	// long alleltslist[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
	// struct Path allelts;
	// allelts.len = 12;
	// allelts.elts = alleltslist;

	// sendcmd('-',offbytes,'-',allelts,list);

	sendcmd2(dir1,dir2,phase,sock);

	// for(int i = 0; i < numpaths; i++){
	// 	free(pathset[i].elts);
	// }
	// free(pathset);
	// free(bytes.contents);

	fprintf(stderr,"About to close comms\n");
	closecomms2(sock);
	fprintf(stderr,"Done closing comms\n");

	return 0;
}
