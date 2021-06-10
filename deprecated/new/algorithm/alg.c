// alg.c -- created by Will Sussman on December 10, 2019

#define PORT 4012
#define TIMEOUT 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int main(int argc, char **argv){
	if(argc != 6){
		fprintf(stderr,"usage: alg topfile startid startdir endid enddir\n");
		return 1;
	}

	char *topfile = argv[1];
	long startid = atol(argv[2]);
	int startdir = tolower(argv[3][0]);
	long endid = atol(argv[4]);
	int enddir = tolower(argv[5][0]);

	struct List *list = NULL;
	makelist(topfile,&list);

	fprintf(stdout,"About to open comms\n");
	opencomms(list,PORT,TIMEOUT);
	fprintf(stdout,"Done opening comms\n");

	int numpaths = 0;
	struct Path path;
	path.len = 0;
	path.elts = NULL;
	struct Path *pathset = NULL;
	// printf("about to genpaths2()\n");
	genpaths2(&path,list,&pathset,startid,endid,&numpaths);
	if(numpaths == 0){
		fprintf(stderr,"No path exists\n");
		return 1;
	}

	int minpath = 0;
	for(int i = 1; i < numpaths; i++){
		if(pathset[i].len < pathset[minpath].len){
			minpath = i;
		}
	}

	struct Buffer bytes;
	// printf("about to prepbytes()\n");
	prepbytes(pathset[minpath],&bytes,list);
	if(startdir == bytes.contents[0]){
		fprintf(stderr,"Invalid startdir\n");
		return 1;
	}
	if(enddir == bytes.contents[bytes.size - 1]){
		fprintf(stderr,"Invalid enddir\n");
		return 1;
	}
	// for(int i = 0; i < bytes.size; i++){
	// 	printf("%c",bytes.contents[i]);
	// }
	// printf("about to sendcmd()\n");
	char offbytes[22] = {'-','-','-','-','-','-','-','-','-','-','-', \
					'-','-','-','-','-','-','-','-','-','-','-'};

	long alleltslist[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
	struct Path allelts;
	allelts.len = 12;
	allelts.elts = alleltslist;

	sendcmd('-',offbytes,'-',allelts,list);

	sendcmd(startdir,bytes.contents,enddir,pathset[minpath],list);

	for(int i = 0; i < numpaths; i++){
		free(pathset[i].elts);
	}
	free(pathset);
	free(bytes.contents);

	fprintf(stderr,"About to close comms\n");
	closecomms(list);
	fprintf(stderr,"Done closing comms\n");

	return 0;
}

// struct Buffer buf = loadf(argv[1]);
// if(buf.size == -1){
// 	fprintf(stderr,"loadf() failed\n");
// 	return 1;
// }
// if(buf.size == 0){
// 	fprintf(stderr,"%s is empty\n",argv[1]);
// 	return 1;
// }

/* To read an integer from row 2, column 3 of 'struct Buffer buf' */
// int beg;
// if((beg = cellstart(2,3,buf)) == -1){
// 	fprintf(stderr,"cellstart() failed\n");
// 	return 1;
// }
// long num = strtol(buf.contents + beg,NULL,10);
// printf("the integer at (2,3) is: %ld\n",num);
// (error check)



/* To read a path from row 1, column 4 of 'struct Buffer buf' */
// int beg2;
// if((beg2 = cellstart(1,4,buf)) == -1){
// 	fprintf(stderr,"cellstart() failed\n");
// 	return 1;
// }
// struct Path path = parsepath(buf,beg2);
// printf("the length of the path at (1,4) is: %d\n",path.len);
// for(int i = 0; i < path.len; i++){
// 	printf("element #%d of the path at (1,4) is: %ld\n",i,path.elts[i]);
// }