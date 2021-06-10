// algorithm.c -- created by Will Sussman on February 3, 2020

#include "/home/riz3/willsLibrary/willslib.h"

int main(int argc, char **argv){
	/* Handle inputs */
	if(argc != 10){
		fprintf(stderr,"usage: algorithm topfile PCport PCtimeout frac thresh listenTrials alertTimeout redundancy neighTrials\n");
		fprintf(stderr,"example: algorithm finaltop.txt 4013 0 0.75 300.0 10 -1 3 10\n");
		exit(1);
	}
	char *filename = argv[1];
	int PCport = atoi(argv[2]);
	float PCtimeout = atof(argv[3]);
	float frac = atof(argv[4]);
	float thresh = atof(argv[5]);
	int listenTrials = atoi(argv[6]);
	float alertDuration = atof(argv[7]);
	int redundancy = atoi(argv[8]);
	int neighTrials = atoi(argv[9]);

	/* Initialize data structure */
	FILE *topfile = fopen(filename,"r");
	if(!topfile){
		perror(filename);
		exit(1);
	}
	struct Topdata topdata;
	loadTop(topfile,&topdata);
	fclose(topfile);

	/* Connect to forwarding PCs */
	printf("Connecting to forwarding PCs...\n");
	struct Host *currhostPtr = topdata.hosts;
	for(int i = 0; i < topdata.nhosts; i++){
		if(!currhostPtr){ // shouldn't happen
			fprintf(stderr,"algorithm: currhostPtr is NULL\n");
			exit(1);
		}
		if((currhostPtr->sock = topdata.hostSocks[i] = openClient(currhostPtr->addr,PCport,PCtimeout)) == -1){
			fprintf(stderr,"algorithm: openClient() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	/* Start listening */
	char triplet[3];
	setTriplet(triplet,'l',(char)listenTrials,'-'); // listen on all antennas listenTrials times each
	genericBroadcast(topdata.hosts,triplet,3);

	/* Detect Rx spike */
	printf("Waiting for alert from any Arduino through its PC...\n");
	int spikeID;
	int stat = waitAlert(topdata,&spikeID,alertDuration);
	if(!stat){
		fprintf(stderr,"algorithm: waitAlert() failed to read\n");
		exit(1);
	} else if(stat == -1){
		fprintf(stderr,"algorithm: waitAlert() failed\n");
		exit(1);
	}

	/* Get line in data structure */
	struct Line *spikePtr;
	if(!(spikePtr = lineLookup(topdata.list,spikeID))){
		fprintf(stderr,"algorithm: lineLookup() failed\n");
		exit(1);
	}

	/* Find best in neighborhood of Rx spike */
	struct Endpoint endpt1;
	if(bestNearby(spikePtr,neighTrials,&endpt1)){
		fprintf(stderr,"algorithm: bestNearby() failed\n");
		exit(1);
	}

	struct Paths allpaths;
	struct Path minpath;
	struct Endpoint endpt2;
	char pathAnt;

	while(1){

		/* Detect Tx spike */
		printf("Waiting for alert from any Arduino through its PC...\n");
		stat = waitAlert(topdata,&spikeID,alertDuration);
		if(!stat){
			fprintf(stderr,"algorithm: waitAlert() failed to read\n");
			exit(1);
		} else if(stat == -1){
			fprintf(stderr,"algorithm: waitAlert() failed\n");
			exit(1);
		}

		/* Get line in data structure */
		if(!(spikePtr = lineLookup(topdata.list,spikeID))){
			fprintf(stderr,"algorithm: lineLookup() failed\n");
			exit(1);
		}

		while(1){
			/* Find best in neighborhood of Tx spike */
			if(bestNearby(spikePtr,neighTrials,&endpt2)){
				fprintf(stderr,"algorithm: bestNearby() failed\n");
				exit(1);
			}
			if(endpt2.data.val < thresh){
				printf("Lost endpoint\n");
				break;
			}

			/* Generate paths between endpoints */
			if(genPaths(&allpaths,topdata.list,endpt2.line->idnum,endpt1.line->idnum,false)){ // use true for disjoint
				fprintf(stderr,"algorithm: genPaths() failed for %d->%d\n",endpt2.line->idnum,endpt1.line->idnum);
				exit(1);
			}
			if(!allpaths.count){
				fprintf(stderr,"algorithm: no such path(s) exist for %d->%d\n",endpt2.line->idnum,endpt1.line->idnum);
				exit(1);
			}

			/* Find shortest available path */
			minpath = allpaths.paths[0];
			for(int i = 1; i < allpaths.count; i++){
				if(allpaths.paths[i].len < minpath.len){
					minpath = allpaths.paths[i];
				}
			}
			printf("Minpath from %d->%d: ",endpt2.line->idnum,endpt1.line->idnum);
			printPath(stdout,minpath);
			// for(int i = 0; i < minpath.len; i++){ // needed for disjoint set
			// 	minpath.nodes[i]->used = true;
			// }

			/* Generate commands for PCs to forward to Arduinos */
			if(prepCmds(topdata.list,endpt2.data.dir,minpath,endpt1.data.dir)){
				fprintf(stderr,"algorithm: prepCmds() failed\n");
				exit(1);
			}

			freePaths(allpaths);

			// currlinePtr = topdata.list;
			// while(currlinePtr){ // needed for new disjoint set
			// 	currlinePtr->used = false;
			// 	currlinePtr = currlinePtr->next;
			// }

			/* Save path antenna of second endpoint (since cleared by sendAllCmds()) */
			pathAnt = endpt2.line->triplet[2]; // output antenna (since Tx)

			/* Send commands to forwarding PCs */
			printf("Sending commands to forwading PCs...\n");
			if(sendAllCmds(topdata.list)){
				fprintf(stderr,"algorithm: sendAllCmds() failed\n");
				exit(1);
			}

			/* Measure second endpoint until deterioration */
			if(getUpdates(endpt2,frac,pathAnt,redundancy)){ // shouldn't return until deterioration detected
				fprintf(stderr,"algorithm: getUpdates() failed\n");
				exit(1);
			}

			spikePtr = endpt2.line;
		}
	}

	/* Disconnect from forwarding PCs */
	printf("Disconnecting from forwarding PCs...\n");
	currhostPtr = topdata.hosts;
	while(currhostPtr){
		if(close(currhostPtr->sock)){
			fprintf(stderr,"setpath: close() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	freeTop(topdata);
	return 0;
}
