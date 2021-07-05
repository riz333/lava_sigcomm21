// setnode.c -- created by Will Sussman on January 1, 2020

#include <stdlib.h>
#include <unistd.h>
#include "/home/riz3/willsLibrary/willslib.h"

int main(int argc, char **argv){

	/* Handle inputs */
	if(argc != 6){
		fprintf(stderr,"usage: setnode filename port timeout idnum ccx\n");
		exit(1);
	}
	char *filename = argv[1];
	int port = atoi(argv[2]);
	float timeout = atof(argv[3]);
	int idnum = atoi(argv[4]);
	char triplet[3] = {argv[5][0],argv[5][1],argv[5][2]};

	/* Initialize data structure */
	FILE *topfile = fopen(filename,"r");
	if(!topfile){
		perror(filename);
		exit(1);
	}
	struct Topdata topdata;
	loadTop(topfile,&topdata);
	fclose(topfile);
	// printTop(stdout,topdata.list);//

	/* Connect to forwarding PCs */
	// struct Host *currhostPtr = topdata.hosts;
	// while(currhostPtr){
	// 	printf("Connecting to %s...\n",currhostPtr->addr);
	// 	if((currhostPtr->sock = openClient(currhostPtr->addr,port,timeout)) == -1){
	// 		fprintf(stderr,"setnode: openClient() failed for %s\n",currhostPtr->addr);
	// 		exit(1);
	// 	}
	// 	currhostPtr = currhostPtr->next;
	// }

	/* Lookup the node and connect to it */
	struct Line *linePtr = lineLookup(topdata.list,idnum);
	if(!linePtr){
		fprintf(stderr,"setnode: no such node\n");
		exit(1);
	}
	printf("Connecting to %s...\n",linePtr->ipaddr);
	int sock = openClient(linePtr->ipaddr,port,timeout);
	if(sock == -1){
		fprintf(stderr,"setnode: openClient() failed for %s\n",linePtr->ipaddr);
		exit(1);
	}

	/* Send command and disconnect */
	printf("Sending (%d:%c%c%c) to %s\n",idnum,triplet[0],triplet[1],triplet[2],linePtr->ipaddr);
	if(sendTriplet(sock,triplet)){
		fprintf(stderr,"setnode: sendTriplet() failed for %s\n",linePtr->ipaddr);
		exit(1);
	}
	printf("Disconnecting from %s...\n",linePtr->ipaddr);
	if(close(sock)){
		fprintf(stderr,"fwdcmds: close() failed for %s\n",linePtr->ipaddr);
		exit(1);
	}

	/* Disconnect from forwarding PCs */
	// currhostPtr = topdata.hosts;
	// while(currhostPtr){
	// 	printf("Disconnecting from %s...\n",currhostPtr->addr);
	// 	if(closeComms(currhostPtr->sock)){
	// 		fprintf(stderr,"setnode: closeComms() failed for %s\n",currhostPtr->addr);
	// 		exit(1);
	// 	}
	// 	currhostPtr = currhostPtr->next;
	// }

	// printf("setpath: about to freeTop(topdata)\n");
	freeTop(topdata);
	return 0;
}
