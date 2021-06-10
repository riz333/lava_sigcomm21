// setoff.c -- created by Will Sussman on January 1, 2020

#include <stdlib.h>
#include <unistd.h>
#include "willslib.h"

int main(int argc, char **argv){

	/* Handle inputs */
	if(argc != 4){
		fprintf(stderr,"usage: setoff filename port timeout\n");
		exit(1);
	}
	char *filename = argv[1];
	int port = atoi(argv[2]);
	float timeout = atof(argv[3]);

	/* Initialize data structure */
	FILE *topfile = fopen(filename,"r");
	if(!topfile){
		perror(filename);
		exit(1);
	}
	struct Topdata topdata;
	loadTop(topfile,&topdata);
	fclose(topfile);
	// printTop(stdout,topdata.list);

	/* Connect to forwarding PCs */
	struct Host *currhostPtr = topdata.hosts;
	while(currhostPtr){
		printf("Connecting to %s...\n",currhostPtr->addr);
		if((currhostPtr->sock = openClient(currhostPtr->addr,port,timeout)) == -1){
			fprintf(stderr,"setpath: openClient() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	/* Prepare commands for each Arduino */
	// printf("setpath: about to sendAllCmds()\n");
	// if(prepOffCmds(topdata.list)){
	// 	fprintf(stderr,"setpath: prepCmds() failed\n");
	// 	exit(1);
	// }
	// printf("setpath: about to freePaths(allpaths)\n");
	// freePaths(allpaths);

	/* Send commands to forwarding PCs */
	if(sendAllCmds(topdata.list)){
		fprintf(stderr,"setpath: sendAllCmds() failed\n");
		exit(1);
	}

	/* Disconnect from forwarding PCs */
	currhostPtr = topdata.hosts;
	while(currhostPtr){
		printf("Disconnecting from %s...\n",currhostPtr->addr);
		if(close(currhostPtr->sock)){
			fprintf(stderr,"setpath: close() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	// printf("setpath: about to freeTop(topdata)\n");
	freeTop(topdata);
	return 0;
}
