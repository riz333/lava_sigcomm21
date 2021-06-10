// fwdcmds2.c -- created by Will Sussman on February 4, 2020

/*

From controller: 6en
Forward en to #6

From controller: l-10, le10 // listen
If -, get median of 10 trials for each antenna on loop until spike detected
If e, get median of 10 trials for antenna e on loop until spike detected
When spike detected, alert controller

From controller: 6g10 // get data
Get median of 10 trials
Send to controller

From controller: 6u10 // get updates
Get median of 10 trials on loop until too low
Alert controller

*/
/*

Initially Arduino is waiting for first command, and its clock is 0
The controller clock for that Arduino is also initially 0
The controller sends it a command, and changes its clock copy to 1
The Arduino receives the command and updates its clock to 1
When the Arduino sends back to the controller, it will send with 1
If the message received at the controller is not 1, throw out

*/

#include "/home/riz3/willsLibrary/willslib.h"

int main(int argc, char **argv){
	/* Handle inputs */
	if(argc != 7){
		fprintf(stderr,"usage: fwdcmds2 topfile topPort topTimeout recPort recTimeout thresh\n");
		fprintf(stderr,"example: fwdcmds2 finaltop.txt 4012 0 4013 0 300.0\n");
		return 1;
	}
	char *filename = argv[1];
	int topPort = atoi(argv[2]);
	float topTimeout = atof(argv[3]);
	int recPort = atoi(argv[4]);
	float recTimeout = atof(argv[5]);
	float thresh = atof(argv[6]);

	/* Initialize data structure */
	FILE *topfile = fopen(filename,"r");
	if(!topfile){
		perror(filename);
		exit(1);
	}
	struct Topdata topdata;
	loadTop(topfile,&topdata);
	fclose(topfile);

	/* Get inet_addr */
	struct ifaddrs *ifap, *currifa;
	if(getifaddrs(&ifap)){
		fprintf(stderr,"fwdcmds: getifaddrs() failed\n");
		exit(1);
	}
	struct sockaddr_in sin;
	currifa = ifap;
	while(currifa){
		sin = *(struct sockaddr_in *)currifa->ifa_addr;
		if(sin.sin_family == AF_INET && strcmp(currifa->ifa_name,"lo") && strcmp(currifa->ifa_name,"enp3s0")){
			topdata.inet_addr = malloc(sizeof(*topdata.inet_addr) * INET_ADDRSTRLEN);
			inet_ntop(AF_INET,&sin.sin_addr,topdata.inet_addr,INET_ADDRSTRLEN);
			break;
		}
		currifa = currifa->ifa_next;
	}
	if(!topdata.inet_addr){
		fprintf(stderr,"fwdcmds: failed to get inet_addr\n");
		exit(1);
	}
	freeifaddrs(ifap);

	/* Connect to Arduinos */
	printf("Connecting to Arduinos...\n");
	struct Line *currlinePtr = topdata.list;
	while(currlinePtr){
		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
			if((currlinePtr->sock = openClient(currlinePtr->ipaddr,topPort,topTimeout)) == -1){
				fprintf(stderr,"fwdcmds: openClient() failed for #%d\n",currlinePtr->idnum);
				currlinePtr->sock = -1;
				fprintf(stderr,"WARNING: Proceeding anyway\n");
			}
		}
		currlinePtr = currlinePtr->next;
	}

	/* Connect to controller */
	printf("Connecting to controller...\n");
	int recSock, listenSock;
	struct sockaddr_in server;
	if((recSock = openServ(&listenSock,recPort,recTimeout,&server)) == -1){
		fprintf(stderr,"fwdcmds: openServ() failed for controller\n");
		exit(1);
	}

	char triplet[3], quad[4], dirs[4] = {'n','e','s','w'}, duo[2];
	int ntrials, stat, spikeID;
	float *antsData, *trialsData, score;
	struct Data bestData;

	while(1){
		/* Get command from controller */
		printf("Waiting for command from controller...\n");
		stat = readAnswer(recSock,triplet,3);
		if(!stat){
			printf("Controller disconnected\n");
			if(close(recSock)){
				fprintf(stderr,"fwdcmds2: close() failed for controller\n");
				exit(1);
			}
			break;
		} else if(stat == -1){
			fprintf(stderr,"fwdcmds2(): readAnswer() failed for controller\n");
			exit(1);
		}

		if(triplet[0] == 'l'){ // first command, starts listening
			printf("Received %c%c%d from controller\n",triplet[0],(int)triplet[1],triplet[2]);
			ntrials = (int)triplet[1];
			duo[1] = '-';
			if(triplet[2] == '-'){ // all antennas
				// currlinePtr = topdata.list;
				// while(currlinePtr){
				// 	if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
				// 		printf("Sending %c%c to #%d\n",triplet[2],'-',currlinePtr->idnum);
				// 		if(genericSend(currlinePtr->sock,(char *)&spikeID,1)){
				// 			fprintf(stderr,"fwdcmds2: genericSend() failed\n");
				// 			exit(1);
				// 		}
				// 	}
				// 	currlinePtr = currlinePtr->next;
				// }
				fprintf(stderr,"NOT YET IMPLEMENTED\n");
			} else { // specific antenna
				duo[0] = triplet[2];
				currlinePtr = topdata.list;
				while(currlinePtr){
					if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
						printf("Sending %c%c to #%d\n",duo[0],duo[1],currlinePtr->idnum);
						if(genericSend(currlinePtr->sock,duo,2)){
							fprintf(stderr,"fwdcmds2: genericSend() failed\n");
							exit(1);
						}
					}
					currlinePtr = currlinePtr->next;
				}
				while(currlinePtr){
					if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
						for(int i = 0; i < ntrials; i++){
							stat = readAnswer(currlinePtr->sock,&trials[i],sizeof(trials[i]));
							if(!stat){
								fprintf(stderr,"fwdcmds2(): readAnswer() failed to read from #%d\n",currlinePtr->idnum);
								exit(1);
							} else if(stat == -1){
								fprintf(stderr,"fwdcmds2(): readAnswer() failed for #%d\n",currlinePtr->idnum);
								exit(1);
							}
						}
						if((median = getMedian2(trials,ntrials)) >= thresh){
							spike.val = median;
							spike.dir = duo[0];
							printf("Sending (%c,%f) to controller\n",spike.dir,spike.val);
							if(genericSend(recSock,&spike,sizeof(spike))){
								fprintf(stderr,"fwdcmds2: genericSend() failed\n");
								exit(1);
							}
							break;
						}
					}
					currlinePtr = currlinePtr->next;
				}
			}
		} else { // forward command to Arduino
			printf("Received %c%c%c from controller\n",triplet[0],triplet[1],triplet[2]);
			currlinePtr = lineLookup(topdata.list,(int)triplet[0]);
			duo[0] = triplet[1]; duo[1] = triplet[2];
			printf("Sending %c%c to #%d\n",duo[0],duo[1],currlinePtr->idnum);
			if(genericSend(currlinePtr->sock,duo,2)){
				fprintf(stderr,"fwdcmds2: genericSend() failed\n");
				exit(1);
			}
		}

		// if((quad[3] >= '0' && quad[3] <= '9') ||
		// 	(quad[3] >= 'a' && quad[3] <= 'f')){
		// 	// forward to (int)quad[0]
		// 	setTriplet(triplet,quad[1],quad[2],quad[3]);
		// 	currlinePtr = lineLookup(topdata.list,(int)quad[0]);
		// 	printf("Sending %c%c%c to #%d\n",triplet[0],triplet[1],triplet[2],currlinePtr->idnum);
		// 	if(sendTriplet(currlinePtr->sock,triplet)){
		// 		fprintf(stderr,"fwdcmds2: sendTriplet() failed for #%d\n",currlinePtr->idnum);
		// 		exit(1);
		// 	}
		// } else if(quad[3] == 'x'){
		// 	// broadcast off
		// 	setTriplet(triplet,'-','-','0');
		// 	currlinePtr = topdata.list;
		// 	while(currlinePtr){
		// 		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
		// 			printf("Sending %c%c%c to #%d\n",triplet[0],triplet[1],triplet[2],currlinePtr->idnum);
		// 			if(sendTriplet(currlinePtr->sock,triplet)){
		// 				fprintf(stderr,"fwdcmds2: sendTriplet() failed for #%d\n",currlinePtr->idnum);
		// 				exit(1);
		// 			}
		// 		}
		// 		currlinePtr = currlinePtr->next;
		// 	}
		// } else if(quad[3] == 'l'){
		// 	// broadcast listening
		// 	setTriplet(triplet,'-','-','l');
		// 	currlinePtr = topdata.list;
		// 	while(currlinePtr){
		// 		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
		// 			printf("Sending %c%c%c to #%d\n",triplet[0],triplet[1],triplet[2],currlinePtr->idnum);
		// 			if(sendTriplet(currlinePtr->sock,triplet)){
		// 				fprintf(stderr,"fwdcmds2: sendTriplet() failed for #%d\n",currlinePtr->idnum);
		// 				exit(1);
		// 			}
		// 		}
		// 		currlinePtr = currlinePtr->next;
		// 	}
		// 	while(1){ // ***
		// 		stat = waitAlert2(topdata,&spikeID,-1);
		// 		if(!stat){
		// 			fprintf(stderr,"fwdcmds2: waitAlert() failed to read\n");
		// 			exit(1);
		// 		} else if(stat == -1){
		// 			fprintf(stderr,"fwdcmds2: waitAlert() failed\n");
		// 			exit(1);
		// 		}

		// 		if(genericSend(recSock,(char *)&spikeID,1)){
		// 			fprintf(stderr,"fwdcmds2: genericSend() failed\n");
		// 			exit(1);
		// 		}
		// 	}
		// } else if(quad[3] == 'g'){
		// 	// get data from (int)quad[0];
		// 	setTriplet(triplet,quad[1],quad[2],'g');
		// 	currlinePtr = lineLookup(topdata.list,(int)quad[0]);
		// 	ntrials = (int)quad[2];
		// 	printf("Sending %c%c%c to #%d\n",triplet[0],triplet[1],triplet[2],currlinePtr->idnum);
		// 	if(sendTriplet(currlinePtr->sock,triplet)){
		// 		fprintf(stderr,"fwdcmds2: sendTriplet() failed for #%d\n",currlinePtr->idnum);
		// 		exit(1);
		// 	}
		// 	trialsData = malloc(sizeof(*trialsData) * ntrials);
		// 	if(!trialsData){
		// 		fprintf(stderr,"fwdcmds2: malloc() failed\n");
		// 		exit(1);
		// 	}
		// 	if(quad[1] == '-'){
		// 		antsData = malloc(sizeof(*antsData) * 4); // note: assumes Arduino tests all 4 directions
		// 		if(!antsData){
		// 			fprintf(stderr,"fwdcmds2: malloc() failed\n");
		// 			exit(1);
		// 		}
		// 		for(int i = 0; i < 4; i++){ // note: assumes Arduino tests all 4 directions
		// 			for(int j = 0; j < ntrials; j++){
		// 				stat = readAnswer(currlinePtr->sock,&trialsData[j],sizeof(trialsData[j]));
		// 				if(!stat){
		// 					fprintf(stderr,"fwdcmds2(): readAnswer() failed to read from #%d\n",currlinePtr->idnum);
		// 					exit(1);
		// 				} else if(stat == -1){
		// 					fprintf(stderr,"fwdcmds2(): readAnswer() failed for #%d\n",currlinePtr->idnum);
		// 					exit(1);
		// 				}
		// 				printf("%d%c trial %d/%d: got %f\n",currlinePtr->idnum,dirs[i],j+1,ntrials,trialsData[j]);
		// 			}
		// 			antsData[i] = getMedian2(trialsData,ntrials);
		// 			printf("%d%c median: %f\n",currlinePtr->idnum,dirs[i],antsData[i]);
		// 			if(!i){
		// 				bestData.val = antsData[0];
		// 				bestData.dir = dirs[0];
		// 			} else if(antsData[i] > bestData.val){
		// 				bestData.val = antsData[i];
		// 				bestData.dir = dirs[i];
		// 			}
		// 			printf("%d best so far: (%c,%f)\n",currlinePtr->idnum,bestData.dir,bestData.val);
		// 		}
		// 	} else {
		// 		for(int j = 0; j < ntrials; j++){
		// 			stat = readAnswer(currlinePtr->sock,&trialsData[j],sizeof(trialsData[j]));
		// 			if(!stat){
		// 				fprintf(stderr,"fwdcmds2: readAnswer() failed to read from #%d\n",currlinePtr->idnum);
		// 				exit(1);
		// 			} else if(stat == -1){
		// 				fprintf(stderr,"fwdcmds2: readAnswer() failed for #%d\n",currlinePtr->idnum);
		// 				exit(1);
		// 			}
		// 			printf("%d%c trial %d/%d: got %f\n",currlinePtr->idnum,quad[1],j+1,ntrials,trialsData[j]);
		// 		}
		// 		bestData.val = getMedian2(trialsData,ntrials);
		// 		bestData.dir = quad[1];
		// 		printf("%d%c median: %f\n",currlinePtr->idnum,quad[1],bestData.val);
		// 	}
		// 	printf("Sending (%c,%f) to controller...\n",bestData.dir,bestData.val);
		// 	if(genericSend(recSock,&bestData,sizeof(bestData))){
		// 		fprintf(stderr,"fwdcmds2: genericSend() failed\n");
		// 		exit(1);
		// 	}
		// } else if(quad[3] == 'u'){
		// 	// get updates from (int)quad[0]
		// 	setTriplet(triplet,quad[1],quad[2],'u');
		// 	currlinePtr = lineLookup(topdata.list,(int)quad[0]);
		// 	printf("Sending %c%c%c to #%d\n",triplet[0],triplet[1],triplet[2],currlinePtr->idnum);
		// 	if(sendTriplet(currlinePtr->sock,triplet)){
		// 		fprintf(stderr,"fwdcmds2: sendTriplet() failed for #%d\n",currlinePtr->idnum);
		// 		exit(1);
		// 	}
		// 	while(1){
		// 		stat = readAnswer(currlinePtr->sock,&score,sizeof(score));
		// 		if(!stat){
		// 			continue;
		// 		} else if(stat == -1){
		// 			fprintf(stderr,"fwdcmds2: readAnswer() failed for #%d\n",currlinePtr->idnum);
		// 			exit(1);
		// 		}
		// 		printf("%d%c%c reported %f\n",currlinePtr->idnum,quad[1],quad[2],score);
		// 		if(gotSignal(recSock)){
		// 			printf("Ignoring last report, ending updates\n");
		// 			//...
		// 			break;
		// 		}
		// 		printf("Sending %f to controller...\n",score);
		// 		if(genericSend(recSock,&score,sizeof(score))){
		// 			fprintf(stderr,"fwdcmds2: genericSend() failed\n");
		// 			exit(1);
		// 		}
		// 	}
		// } else {
		// 	fprintf(stderr,"fwdcmds2: invalid quad[3]\n");
		// 	exit(1);
		// }
	}

	/* Disconnect from controller */
	printf("Disconnecting from controller...\n");
	if(close(listenSock)){
		fprintf(stderr,"fwdcmds: close() failed for controller\n");
		exit(1);
	}

	/* Disconnect from Arduinos */
	printf("Disconnecting from Arduinos...\n");
	currlinePtr = topdata.list;
	while(currlinePtr){
		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr) && currlinePtr->sock != -1){
			// printf("Disconnecting from %s...\n",currlinePtr->ipaddr);
			if(close(currlinePtr->sock)){
				fprintf(stderr,"fwdcmds: close() failed for #%d\n",currlinePtr->idnum);
				exit(1);
			}
		}
		currlinePtr = currlinePtr->next;
	}

	freeTop(topdata);
	return 0;
}
