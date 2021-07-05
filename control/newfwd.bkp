// newfwd.c -- created by Will Sussman on February 7, 2020

#define _GNU_SOURCE

#include "/home/riz3/willsLibrary/willslib.h"

int main(int argc, char **argv){

	/* Handle inputs */
	if(argc != 8){
		fprintf(stderr,"usage: newfwd topfile topPort topTimeout conPort conTimeout alertTimeout parent_addr\n");
		fprintf(stderr,"example: newfwd finaltop.txt 4012 0 4013 0 -1 172.29.220.222\n");
		return 1;
	}
	char *filename = argv[1];
	int topPort = atoi(argv[2]);
	float topTimeout = atof(argv[3]);
	int conPort = atoi(argv[4]);
	float conTimeout = atof(argv[5]);
	float alertTimeout = atof(argv[6]);
	char *parent_addr = argv[7];

	/* Initialize data structure */
	FILE *topfile = fopen(filename,"r");
	if(!topfile){
		perror(filename);
		exit(1);
	}
	struct Topdata topdata;
	if(loadTop(topfile,&topdata)){
		fprintf(stderr,"newfwd: loadTop() failed\n");
		exit(1);
	}
	topdata.alertTimeout = alertTimeout;
	topdata.gotCommand = false;
	// topdata.alpha = alpha;
	fclose(topfile);

	/* Get inet_addr */
	struct ifaddrs *ifap, *currifa;
	if(getifaddrs(&ifap)){
		fprintf(stderr,"newfwd: getifaddrs() failed\n");
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
		fprintf(stderr,"newfwd: failed to get inet_addr\n");
		exit(1);
	}
	freeifaddrs(ifap);

	/* Set topdata.nlocal */
	struct Line *currlinePtr = topdata.list;
	while(currlinePtr){
		// printf("currently #%d\n",currlinePtr->idnum);
		// printf("host: %s\n",currlinePtr->host->addr);
		// printf("self: %s\n",topdata.inet_addr);
		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
			topdata.nlocal++;
		}
		currlinePtr = currlinePtr->next;
	}
	// printf("nlocal is now: %d\n",topdata.nlocal);

	topdata.localSocks = malloc(sizeof(*topdata.localSocks) * topdata.nlocal);
	if(!topdata.localSocks){
		fprintf(stderr,"newfwd: malloc() failed\n");
		return 1;
	}

	/* Connect to Arduinos */
	printf("Connecting to Arduinos...\n");
	int listenSock2;
	struct sockaddr_in server2;
	currlinePtr = topdata.list;
	bool gotFirst = false;
	// int i = 0;
	topdata.ardSocks = malloc(sizeof(*topdata.ardSocks) * topdata.nlocal);
	if(openServ2(&listenSock2,topPort,topTimeout,&server2,topdata)){
		fprintf(stderr,"newfwd: openServ() failed for Arduinos\n");
		exit(1);
	}
	printf("Done connecting to Arduinos\n");
	while(currlinePtr){
		if(!currlinePtr){ // shouldn't happen
			fprintf(stderr,"newfwd: currlinePtr is NULL\n");
			exit(1);
		}
		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
			// printf("Trying #%d\n",currlinePtr->idnum);

			/*
			printf("Connecting to Arduino #%d...\n",currlinePtr->idnum);
			if((currlinePtr->sock = topdata.localSocks[i] = openServ(&currlinePtr->listenSock,topPort,topTimeout,&currlinePtr->server)) == -1){
				fprintf(stderr,"newfwd: openServ() failed for #%d\n",currlinePtr->idnum);
				// fprintf(stderr,"WARNING: proceeding anyway\n");
				// currlinePtr = currlinePtr->next;
				// continue;
				exit(1);
			}
			i++;
			*/

			// if((currlinePtr->sock = topdata.localSocks[i++] = openClient(currlinePtr->ipaddr,topPort,topTimeout)) == -1){
			// 	fprintf(stderr,"newfwd: openClient() failed for #%d\n",currlinePtr->idnum);
			// 	fprintf(stderr,"WARNING: proceeding anyway\n");
			// 	currlinePtr = currlinePtr->next;
			// 	continue;
			// 	// exit(1);
			// }
			// printf("#%d fd: %d\n",currlinePtr->idnum,currlinePtr->sock);
			if(!gotFirst){
				gotFirst = true;
				topdata.maxLocal = currlinePtr->sock;
			} else if(currlinePtr->sock > topdata.maxLocal){
				topdata.maxLocal = currlinePtr->sock;
			}
		}
		currlinePtr = currlinePtr->next;
	}
	// printf("Done\n");

	/* Connect to controller */
	printf("Connecting to controller...\n");
	// int listenSock;
	// struct sockaddr_in server;
	// if((topdata.parentSock = openServ(&listenSock,conPort,conTimeout,&server)) == -1){
	// 	fprintf(stderr,"newfwd: openServ() failed for controller\n");
	// 	exit(1);
	// }
	if((topdata.parentSock = openClient(parent_addr,conPort,conTimeout)) == -1){
		fprintf(stderr,"newfwd: openClient() failed for %s\n",parent_addr);
		exit(1);
	}
	printf("Controller socket: %d\n",topdata.parentSock);

	// printf("Done\n");

	/* Update table in background */
	pthread_t table_thread;
	pthread_create(&table_thread,NULL,updateTable,&topdata);

	// char *line;
	// size_t size;
	// char in, out, hex;
	// int idnum;
	// char triplet[3];
	int stat;
	struct Cmd cmd;

	while(1){

		/* Get command from controller */
		printf("Waiting for command...\n");
		stat = recCmd(topdata.parentSock,&cmd);
		if(!stat){
			printf("Controller disconnected\n");
			if(close(topdata.parentSock)){
				fprintf(stderr,"fwdcmds: close() failed for controller\n");
				exit(1);
			}
			break;

			/* Reconnect to controller */
			// printf("Reconnecting to controller...\n");
			// if((recSock = reopenServ(listenSock,&server)) == -1){
			// 	fprintf(stderr,"fwdcmds: reopenServ() failed for controller\n");
			// 	exit(1);
			// }
		} else if(stat == -1){
			fprintf(stderr,"newfwd: recCmd() failed\n");
			exit(1);
		} else {
			/* Forward command to Arduino */
			topdata.gotCommand = true;
			currlinePtr = lineLookup(topdata.list,cmd.idnum);
			if(!currlinePtr){
				fprintf(stderr,"fwdcmds: lineLookup() failed for node %d\n",cmd.idnum);
				exit(1);
			}

			if(currlinePtr->sock == -1){
				printf("Would send (%c%c%c) to %s\n",cmd.triplet[0],cmd.triplet[1],cmd.triplet[2],currlinePtr->ipaddr);
			} else {
				printf("Sending (%c%c%c) to %s\n",cmd.triplet[0],cmd.triplet[1],cmd.triplet[2],currlinePtr->ipaddr);
				if(sendTriplet(currlinePtr->sock,cmd.triplet)){
					fprintf(stderr,"fwdcmds: sendCmd() failed for %s\n",currlinePtr->ipaddr);
					fprintf(stderr,"WARNING: Proceeding anyway\n");
					// exit(1);
				}
			}
		}

		// printf("Enter <in><out><hex><idnum>\n");
		// line = NULL;
		// size = 0;
		// if(getline(&line,&size,stdin) == -1){
		// 	fprintf(stderr,"newfwd: getline() failed\n");
		// 	exit(1);
		// }
		// in = line[0];
		// out = line[1];
		// hex = line[2];
		// idnum = atoi(&line[3]);
		// free(line);

		// currlinePtr = lineLookup(topdata.list,idnum);
		// if(!currlinePtr){
		// 	fprintf(stderr,"newfwd: lineLookup() failed\n");
		// 	exit(1);
		// }

		// setTriplet(triplet,in,out,hex);
		// if(sendTriplet(currlinePtr->sock,triplet)){
		// 	fprintf(stderr,"newfwd: sendTriplet() failed\n");
		// 	exit(1);
		// }
	}

	pthread_cancel(table_thread);
	// topdata.stop = true;
	// pthread_join(table_thread);

	// struct Message message;
	// message.idnum = 4;
	// message.score = 9.2;
	// while(!genericSend(conSock,&message,sizeof(message)));

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

	/* Disconnect from controller */
	printf("Disconnecting from controller...\n");
	// Close conSock, listenSock ???
	if(close(topdata.parentSock)){
		fprintf(stderr,"newfwd: close() failed for controller\n");
		exit(1);
	}

	/* Disconnect from Arduinos */
	printf("Disconnecting from Arduinos...\n");
	currlinePtr = topdata.list;
	while(currlinePtr){
		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr) && currlinePtr->sock != -1){
			// printf("Disconnecting from %s...\n",currlinePtr->ipaddr);
			if(close(currlinePtr->listenSock)){
				fprintf(stderr,"newfwd: close() failed for #%d\n",currlinePtr->idnum);
				exit(1);
			}
			// if(close(currlinePtr->sock)){
			// 	fprintf(stderr,"newfwd: close() failed for %s\n",currlinePtr->ipaddr);
			// 	exit(1);
			// }
		}
		currlinePtr = currlinePtr->next;
	}

	freeTop(topdata);

	return 0;
}