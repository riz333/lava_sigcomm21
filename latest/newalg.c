// newalg.c -- created by Will Sussman on February 7, 2020

// #define _GNU_SOURCE

#include "/home/riz3/willsLibrary/willslib.h"

int main(int argc, char **argv){

	/* Handle inputs */
	int reqArgs = 6;
	if(argc < reqArgs){
		// fprintf(stderr,"usage: newalg topfile PCport PCtimeout alertTimeout alpha\n");
		// fprintf(stderr,"example: newalg finaltop.txt 4013 0 -1 0.90\n");
		fprintf(stderr,"usage: newalg topfile PCport PCtimeout alertTimeout timeBoundary (<idnum>:<in><out><phase>)*\n");
		fprintf(stderr,"example: newalg finaltop.txt 4013 0 -1 5.0 17:nw0 2:es4\n");
		exit(1);
	}
	char *filename = argv[1];
	int PCport = atoi(argv[2]);
	float PCtimeout = atof(argv[3]);
	float alertTimeout = atof(argv[4]);
	float timeBoundary = atof(argv[5]);
	if(timeBoundary < 0.0){
		fprintf(stderr,"newalg: timeBoundary must be positive\n");
		exit(1);
	}
	int nmanual = argc - reqArgs;

	// float alpha = atof(argv[5]);
	// printf("topfile: %s, PCport: %d, PCtimeout: %f, alertTimeout: %f, alpha: %f\n",filename,PCport,PCtimeout,alertTimeout,alpha);

	/* Initialize data structure */
	FILE *topfile = fopen(filename,"r");
	if(!topfile){
		perror(filename);
		exit(1);
	}
	struct Topdata topdata;
	if(loadTop(topfile,&topdata)){
		fprintf(stderr,"newalg: loadTop() failed\n");
		exit(1);
	}
	topdata.alertTimeout = alertTimeout;
	// topdata.alpha = alpha;
	fclose(topfile);


	topdata.timeBoundary.tv_sec = (int)timeBoundary;
	timeBoundary = (timeBoundary - topdata.timeBoundary.tv_sec) * 1000000;
	topdata.timeBoundary.tv_usec = (int)timeBoundary;
	/*
	struct timeval *alertTimeout = NULL;
	struct timeval tv;
	if(alertDuration >= 0){
		// alertTimeout = malloc(sizeof(*alertTimeout));
		tv.tv_sec = (int)alertDuration;
		alertDuration = (alertDuration - tv.tv_sec) * 1000000;
		tv.tv_usec = (int)alertDuration;
		alertTimeout = &tv;
	}
	*/




	/* Connect to forwarding PCs */
	printf("Connecting to forwarding PCs...\n");
	int listenSock3;
	struct sockaddr_in server3;
	if(openServ3(&listenSock3,PCport,PCtimeout,&server3,topdata)){
		fprintf(stderr,"newalg: openServ() failed for PCs\n");
		exit(1);
	}
	printf("Done connecting to PCs\n");

	struct Host *currhostPtr = topdata.hosts;
	for(int i = 0; i < topdata.nhosts; i++){
		if(!currhostPtr){ // shouldn't happen
			fprintf(stderr,"newalg: currhostPtr is NULL\n");
			exit(1);
		}
	// 	if((currhostPtr->sock = topdata.hostSocks[i] = openClient(currhostPtr->addr,PCport,PCtimeout)) == -1){
	// 		fprintf(stderr,"newalg: openClient() failed for %s\n",currhostPtr->addr);
	// 		exit(1);
	// 	}
		if(!i){
			topdata.maxHost = currhostPtr->sock;
		} else if(currhostPtr->sock > topdata.maxHost){
			topdata.maxHost = currhostPtr->sock;
		}
		currhostPtr = currhostPtr->next;
	}

	if(setAll("e-0",topdata.list)){
		fprintf(stderr,"newalg: setAll() failed\n");
		exit(1);
	}
	
	pthread_t table_thread;
	if(nmanual){
		/* Update table in background */
		pthread_create(&table_thread,NULL,updateMaster,&topdata);
	} else {
		for(int i = 0; i < nmanual; i++){
			char *rest;
			int idnum = (int)strtol(argv[reqArgs + i],&rest,10);
			char triplet[3];
			triplet[0] = rest[1];
			triplet[1] = rest[2];
			triplet[2] = rest[3];
			struct Line *linePtr = lineLookup(topdata.list,idnum);
			if(sendQuad(linePtr->host->sock,idnum,triplet)){
				fprintf(stderr,"newalg: sendQuad() failed\n");
				exit(1);
			}
		}
	}

	char *line = NULL;
	size_t size = 0;

	// while(1){
	printf("Enter x to finish\n");
	getline(&line,&size,stdin);
	while(line[0] != 'x'){
		free(line);
		line = NULL;
		size = 0;
		getline(&line,&size,stdin);
	}
	free(line);

	// }

	if(nmanual){
		pthread_cancel(table_thread);
	}

	if(setAll("e-0",topdata.list)){
		fprintf(stderr,"newalg: setAll() failed\n");
		exit(1);
	}

	// struct Line *currlinePtr;
	// char in, out;
	// int idnum;
	// char *line;
	// size_t size;
	// while(1){
	// 	// printf("Foreground\n");
	// 	line = NULL;
	// 	size = 0;
	// 	printf("Enter <in><out><idnum>\n");
	// 	if(getline(&line,&size,stdin) == -1){
	// 		break;
	// 	}
	// 	in = line[0];
	// 	out = line[1];
	// 	idnum = atoi(&line[2]);
	// 	free(line);

	// 	currlinePtr = lineLookup(topdata.list,idnum);
	// 	printf("Sending %c%c to #%d\n",in,out,currlinePtr->idnum);

	// }

	/* Disconnect from forwarding PCs */
	printf("Disconnecting from forwarding PCs...\n");
	currhostPtr = topdata.hosts;
	while(currhostPtr){
		if(close(currhostPtr->sock)){
			fprintf(stderr,"newalg: close() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	freeTop(topdata);
	return 0;
}
