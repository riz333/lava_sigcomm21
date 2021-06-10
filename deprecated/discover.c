// discover.c -- created by Will Sussman on January 30, 2020

#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>
#include "/home/riz3/willsLibrary/willslib.h"

int main(int argc, char **argv){

	/* Handle inputs */
	if(argc != 6){
		fprintf(stderr,"usage: discover topfile port timeout ntrials dir\n");
		fprintf(stderr,"example: discover jan16top.txt 4013 0 10 e\n");
		exit(1);
	}
	char *filename = argv[1];
	int port = atoi(argv[2]);
	float timeout = atof(argv[3]);
	int ntrials = atoi(argv[4]);
	char dir = argv[5][0];

	/* Get inet_addr */
	// struct ifaddrs *ifap, *currifa;
	// if(getifaddrs(&ifap)){
	// 	fprintf(stderr,"discover: getifaddrs() failed\n");
	// 	exit(1);
	// }
	// struct sockaddr_in sin;
	// char *inet_addr = NULL;
	// currifa = ifap;
	// while(currifa){
	// 	sin = *(struct sockaddr_in *)currifa->ifa_addr;
	// 	if(sin.sin_family == AF_INET && strcmp(currifa->ifa_name,"lo") && strcmp(currifa->ifa_name,"enp3s0")){
	// 		inet_addr = malloc(sizeof(*inet_addr) * INET_ADDRSTRLEN);
	// 		inet_ntop(AF_INET,&sin.sin_addr,inet_addr,INET_ADDRSTRLEN);
	// 		break;
	// 	}
	// 	currifa = currifa->ifa_next;
	// }
	// if(!inet_addr){
	// 	fprintf(stderr,"discover: failed to get inet_addr\n");
	// 	exit(1);
	// }
	// freeifaddrs(ifap);

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

	/* Connect to Arduinos */
	// printf("Connecting to Arduinos...\n");
	// struct Line *currlinePtr = topdata.list;
	// while(currlinePtr){
	// 	if(!strcmp(currlinePtr->host->addr,inet_addr)){
	// 		// printf("Connecting to %s...\n",currlinePtr->ipaddr);
	// 		if((currlinePtr->sock = openClient(currlinePtr->ipaddr,port,timeout)) == -1){
	// 			fprintf(stderr,"discover: openClient() failed for %s\n",currlinePtr->ipaddr);
	// 			currlinePtr->sock = -1;
	// 			fprintf(stderr,"WARNING: Proceeding anyway\n");
	// 			// exit(1);
	// 		}
	// 	}
	// 	currlinePtr = currlinePtr->next;
	// }

	/* Connect to controller */
	// printf("Connecting to controller...\n");
	// int recSock, listenSock;
	// struct sockaddr_in server;
	// if((recSock = openServ(&listenSock,recPort,recTimeout,&server)) == -1){
	// 	fprintf(stderr,"discover: openServ() failed for controller\n");
	// 	exit(1);
	// }

	/* Connect to forwarding PCs */
	printf("Connecting to forwarding PCs...\n");
	struct Host *currhostPtr = topdata.hosts;
	while(currhostPtr){
		// printf("Connecting to %s...\n",currhostPtr->addr);
		if((currhostPtr->sock = openClient(currhostPtr->addr,port,timeout)) == -1){
			fprintf(stderr,"discover: openClient() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	/* Testing */
	// char buf[10];
	// int nrec;
	
	int stat;//, idnumV, idnumP;//, idMax;
	// float maxVoltage;
	// struct Cmd cmd;
	// cmd.triplet = malloc(sizeof(*cmd.triplet) * 3);
	// printf("Entering while loop\n");
	// char vDir, pDir;
	// float **vData, **pData, vMax, pMax, vAvg, pAvg;
	// struct Data dataBlocks[ntrials];///*vBest, pBest, */vBlock, pBlock;
	// struct Line *bestLinePtr;
	// bool gotFirst, gotSecond;
	char request[3];
	struct Report best, report, bestReport;
	memset(&best,0,sizeof(best));

	currhostPtr = topdata.hosts;
	while(currhostPtr){
		printf("Requesting d report from host %s...\n",currhostPtr->addr);
		request[0] = dir; request[1] = '-'; request[2] = 'd';
		if(sendQuad(currhostPtr->sock,ntrials,request)){
			fprintf(stderr,"getReport(): sendQuad() failed\n");
			exit(1);
		}
		// stat = getReport(currhostPtr->sock,ntrials,dir,'-','d',&report);
		// // printf("Done\n");
		// if(!stat){
		// 	fprintf(stderr,"discover: getReport() failed to read from %s\n",currhostPtr->addr);
		// 	exit(1);
		// } else if(stat == -1){
		// 	fprintf(stderr,"discover: getReport() failed for %s\n",currhostPtr->addr);
		// 	exit(1);
		// }

		// if(currhostPtr == topdata.hosts){
		// 	bestReport = report;
		// } else {
		// 	if(report.data.val > bestReport.data.val){
		// 		bestReport = report;
		// 	}
		// }

		currhostPtr = currhostPtr->next;
	}

	currhostPtr = topdata.hosts;
	while(currhostPtr){
		printf("Getting d report from host %s...\n",currhostPtr->addr);
		// request[0] = dir; request[1] = '-'; request[2] = 'd';
		// if(sendQuad(currhostPtr->sock,ntrials,request)){
		// 	fprintf(stderr,"getReport(): sendQuad() failed\n");
		// 	exit(1);
		// }
		stat = getReport(currhostPtr->sock,&report);
		// printf("Done\n");
		if(!stat){
			fprintf(stderr,"discover: getReport() failed to read from %s\n",currhostPtr->addr);
			exit(1);
		} else if(stat == -1){
			fprintf(stderr,"discover: getReport() failed for %s\n",currhostPtr->addr);
			exit(1);
		}

		if(currhostPtr == topdata.hosts){
			bestReport = report;
		} else {
			if(report.data.val > bestReport.data.val){
				bestReport = report;
			}
		}

		currhostPtr = currhostPtr->next;
	}

	printf("Report: %d %c %f\n",bestReport.idnum,bestReport.data.dir,bestReport.data.val);

	struct Line *currlinePtr = lineLookup(topdata.list,bestReport.idnum);

	printf("Requesting final d report from host %s...\n",currlinePtr->host->addr);
	request[0] = '-'; request[1] = (char)bestReport.idnum; request[2] = 'd';
	if(sendQuad(currhostPtr->sock,ntrials,request)){
		fprintf(stderr,"getReport(): sendQuad() failed\n");
		exit(1);
	}
	// stat = getReport(currlinePtr->host->sock,ntrials,'-',(char)bestReport.idnum,'d',&bestReport);
	// // printf("Done\n");
	// if(!stat){
	// 	fprintf(stderr,"setpath: getReport() failed to read from %s\n",currlinePtr->host->addr);
	// 	exit(1);
	// } else if(stat == -1){
	// 	fprintf(stderr,"setpath: getReport() failed for %s\n",currlinePtr->host->addr);
	// 	exit(1);
	// }

	printf("Getting final d report from host %s...\n",currlinePtr->host->addr);
	stat = getReport(currlinePtr->host->sock,&bestReport);
	// printf("Done\n");
	if(!stat){
		fprintf(stderr,"setpath: getReport() failed to read from %s\n",currlinePtr->host->addr);
		exit(1);
	} else if(stat == -1){
		fprintf(stderr,"setpath: getReport() failed for %s\n",currlinePtr->host->addr);
		exit(1);
	}

	printf("Report: #%d(%c,%f)\n",bestReport.idnum,bestReport.data.dir,bestReport.data.val);


	// memset(&report,0,sizeof(report));

	// printf("Getting commands...\n");
	// while(1){

		/* Get command from controller */
		// stat = recCmd(recSock,&cmd);
		// if(!stat){
		// 	printf("Controller disconnected\n");
		// 	if(close(recSock)){
		// 		fprintf(stderr,"discover: close() failed for controller\n");
		// 		exit(1);
		// 	}

		// 	break;

		// 	/* Reconnect to controller */
		// 	// printf("Reconnecting to controller...\n");
		// 	// if((recSock = reopenServ(listenSock,&server)) == -1){
		// 	// 	fprintf(stderr,"discover: reopenServ() failed for controller\n");
		// 	// 	exit(1);
		// 	// }
		// } else if(stat == -1){
		// 	fprintf(stderr,"discover: recCmd() failed\n");
		// 	exit(1);
		// } else {
		// 	printf("Received (%d:%c%c%c) from controller\n",cmd.idnum,cmd.triplet[0],cmd.triplet[1],cmd.triplet[2]);
		// }

		// if(cmd.triplet[2] == 'v'){

			// if(cmd.idnum == 0){ // query all units

	/*
	currlinePtr = topdata.list;
	gotFirst = false;
	gotSecond = false;
	while(currlinePtr){
		if(!strcmp(currlinePtr->host->addr,inet_addr)){

			printf("\n");

			if(gotFirst){
				gotSecond = true;
			}
			gotFirst = true;

			if(currlinePtr->sock == -1){
				printf("Would query #%d with %c-v\n",currlinePtr->idnum,dir);
				currlinePtr = currlinePtr->next;
				continue;
			}

			for(int i = 0; i < ntrials; i++){
				printf("Querying #%d with %c-v (%d/%d)\n",currlinePtr->idnum,dir,i+1,ntrials);
				stat = queryUnit(currlinePtr->sock,dir,&dataBlocks[i]);
				if(!stat){
					fprintf(stderr,"discover: queryUnit() failed to read from %s\n",currlinePtr->ipaddr);
					exit(1);
				} else if(stat == -1){
					fprintf(stderr,"discover: queryUnit() failed\n");
					exit(1);
				} else {
					printf("Received (%c,%f) from unit #%d\n",dataBlocks[i].dir,dataBlocks[i].val,currlinePtr->idnum);
				}
			}

			median = getMedian(dataBlocks,currlinePtr->idnum,ntrials);

			if(!gotSecond){
				best = median;
				bestLinePtr = currlinePtr;
			} else {
				if(median.data.val > best.data.val){
					best = median;
					bestLinePtr = currlinePtr;
				}
			}

		}
		currlinePtr = currlinePtr->next;
	}

	printf("\nBest %c: %d %f\n",best.data.dir,best.idnum,best.data.val);
	*/

			// } else { // query specific unit

		// currlinePtr = lineLookup(topdata.list,best.idnum);
		// if(!currlinePtr){
		// 	fprintf(stderr,"fwdcmd: lineLookup() failed for node %d\n",best.idnum);
		// 	exit(1);
		// }

		// best.idnum = currlinePtr->idnum;

	/*
	for(int i = 0; i < bestLinePtr->ants.num; i++){

		printf("\n");

		for(int j = 0; j < ntrials; j++){
			printf("Querying #%d with %c-v (%d/%d)\n",bestLinePtr->idnum,bestLinePtr->ants.array[i],j+1,ntrials);
			stat = queryUnit(bestLinePtr->sock,bestLinePtr->ants.array[i],&dataBlocks[j]);
			if(!stat){
				fprintf(stderr,"discover: queryUnit() failed to read from %s\n",bestLinePtr->ipaddr);
				exit(1);
			} else if(stat == -1){
				fprintf(stderr,"discover: queryUnit() failed\n");
				exit(1);
			} else {
				printf("Received (%c,%f) from unit #%d\n",dataBlocks[j].dir,dataBlocks[j].val,bestLinePtr->idnum);
			}
		}

		median = getMedian(dataBlocks,bestLinePtr->idnum,ntrials);

		if(!i){
			best = median;
			// bestLinePtr = currlinePtr;
		} else {
			if(median.data.val > best.data.val){
				best = median;
				// bestLinePtr = currlinePtr;
			}
		}
	}

	printf("\nBest: %d %c %f\n\n",best.idnum,best.data.dir,best.data.val);
	*/

			// }

			// if(report(recSock,best)){
			// 	fprintf(stderr,"discover: report() failed\n");
			// 	exit(1);
			// }

			/*
			currlinePtr = topdata.list;
			gotFirst = false;
			gotSecond = false;

			while(currlinePtr){

				// if(currlinePtr->idnum > 12){
				// 	break;
				// }

				if(!strcmp(currlinePtr->host->addr,inet_addr)){
					if(gotFirst){
						gotSecond = true;
					}
					gotFirst = true;
					// vData = malloc(sizeof(*vData) * currlinePtr->ants.num);
					// pData = malloc(sizeof(*pData) * currlinePtr->ants.num);
					// for(int i = 0; i < currlinePtr->ants.num; i++){
					// 	printf("\nTesting #%d.%d(/%d), which is %c\n",currlinePtr->idnum,i,currlinePtr->ants.num,currlinePtr->ants.array[i]);
					// 	vData[i] = malloc(sizeof(*vData) * nTrials);
					// 	pData[i] = malloc(sizeof(*pData) * nTrials);
					// 	for(int j = 0; j < nTrials; j++){
					// 		stat = queryVoltage(currlinePtr->sock,currlinePtr->ants.array[i],&vData[i][j],&pData[i][j]);
					// 		if(!stat){
					// 			fprintf(stderr,"discover: queryVoltage() failed to read from %s\n",currlinePtr->ipaddr);
					// 			exit(1);
					// 		} else if(stat == -1){
					// 			fprintf(stderr,"discover: queryVoltage() failed\n");
					// 			exit(1);
					// 		} else {
					// 			printf("Trial #%d: voltage of %f V, power of %f\n",j,vData[i][j],pData[i][j]);
					// 		}
					// 	}
					// 	vAvg = average(vData[i],nTrials);
					// 	pAvg = average(pData[i],nTrials);
					// 	printf("vAvg is %f, pAvg is %f\n",vAvg,pAvg);
					// 	free(vData[i]);
					// 	free(pData[i]);

					// 	if(!i){
					// 		vMax = vAvg;
					// 		pMax = pAvg;
					// 		vDir = currlinePtr->ants.array[0];
					// 		pDir = currlinePtr->ants.array[0];
					// 	} else {
					// 		if(vAvg > vMax){
					// 			vMax = vAvg;
					// 			vDir = currlinePtr->ants.array[i];
					// 		}
					// 		if(pAvg > pMax){
					// 			pMax = pAvg;
					// 			pDir = currlinePtr->ants.array[i];
					// 		}
					// 	}
					// }
					// free(vData);
					// free(pData);

					stat = queryBestVoltage(currlinePtr->sock,&vBlock,&pBlock);
					if(!stat){
						fprintf(stderr,"discover: queryVoltage() failed to read from %s\n",currlinePtr->ipaddr);
						exit(1);
					} else if(stat == -1){
						fprintf(stderr,"discover: queryVoltage() failed\n");
						exit(1);
					} else {
						// printf("#%d returned V:(%c,%f) P:(%c,%f)\n",currlinePtr->idnum,vBlock.dir,vBlock.val,pBlock.dir,pBlock.val);
						printf("%d V: %c %f; P: %c %f\n",currlinePtr->idnum,vBlock.dir,vBlock.val,pBlock.dir,pBlock.val);
					}

					// currlinePtr->voltage = vBlock;
					// currlinePtr->power = pBlock;

					if(!gotSecond){
						report.vMax = vBlock;
						report.idnumV = currlinePtr->idnum;
						report.pMax = pBlock;
						report.idnumP = currlinePtr->idnum;
					} else {
						if(vBlock.val > report.vMax.val){
							report.vMax = vBlock;
							report.idnumV = currlinePtr->idnum;
						}
						if(pBlock.val > report.pMax.val){
							report.pMax = pBlock;
							report.idnumP = currlinePtr->idnum;
						}
					}

					// if(!gotFirst){

					// }
					// if(currlinePtr == topdata.list){
					// 	maxVoltage = currlinePtr->voltage;
					// 	idMax = currlinePtr->idnum;
					// } else {
					// 	if(currlinePtr->voltage > maxVoltage){
					// 		maxVoltage = currlinePtr->voltage;
					// 		idMax = currlinePtr->idnum;
					// 	}
					// }
				}
				currlinePtr = currlinePtr->next;
			}

			// struct Report report;
			// report.idnumV = idnumV;
			// report.idnumP = idnumP;
			// report.v = vBest;
			// report.p = pBest;

			// printf("\nReport contains: (V) elt %d, ant %c, val %f\n",report.idnumV,report.vMax.dir,report.vMax.val);
			// printf("Report contains: (P) elt %d, ant %c, val %f\n",report.idnumP,report.pMax.dir,report.pMax.val);
			// printf("Sending to controller\n");

			if(reportMax(recSock,report)){
				fprintf(stderr,"discover: reportMax() failed\n");
				exit(1);
			}
			*/

		// } else {

			/* Forward command to Arduino */
			// currlinePtr = lineLookup(topdata.list,cmd.idnum);
			// if(!currlinePtr){
			// 	fprintf(stderr,"fwdcmd: lineLookup() failed for node %d\n",cmd.idnum);
			// 	exit(1);
			// }

			// if(currlinePtr->sock == -1){
			// 	printf("Would send (%c%c%c) to %s\n",cmd.triplet[0],cmd.triplet[1],cmd.triplet[2],currlinePtr->ipaddr);
			// } else {
			// 	printf("Sending (%c%c%c) to %s\n",cmd.triplet[0],cmd.triplet[1],cmd.triplet[2],currlinePtr->ipaddr);
			// 	if(sendTriplet(currlinePtr->sock,cmd.triplet)){
			// 		fprintf(stderr,"discover: sendCmd() failed for %s\n",currlinePtr->ipaddr);
			// 		fprintf(stderr,"WARNING: Proceeding anyway\n");
			// 		// exit(1);
			// 	}
			// }

		// }

		/* Testing */
		// printf("Press enter to recv()\n");
		// getchar();
		// printf("About to recv()\n");
		// nrec = recv(recSock,buf,10,0);
		// if(nrec == -1){
		// 	printf("recv() failed\n");
		// } else if(nrec == 0){
		// 	printf("disconnected\n");
		// } else {
		// 	printf("contents of buf: %s\n",buf);
		// }
		// printf("Press enter to read()\n");
		// getchar();
		// printf("About to read()\n");
		// nrec = read(recSock,buf,10);
		// if(nrec == -1){
		// 	printf("read() failed\n");
		// } else {
		// 	printf("nrec %d\n",nrec);
		// }

		// printf("Press enter to loop\n");
		// getchar();

	// }

	// free(cmd.triplet);

	// printf("Exiting while loop\n");

	// printf("Press enter to finish\n");
	// getchar();

	/* Disconnect from controller */
	// printf("Disconnecting from controller...\n");
	// if(close(listenSock)){
	// 	fprintf(stderr,"discover: close() failed for controller\n");
	// 	exit(1);
	// }

	/* Disconnect from Arduinos */
	// printf("Disconnecting from Arduinos...\n");
	// currlinePtr = topdata.list;
	// while(currlinePtr){
	// 	if(!strcmp(currlinePtr->host->addr,inet_addr) && currlinePtr->sock != -1){
	// 		// printf("Disconnecting from %s...\n",currlinePtr->ipaddr);
	// 		if(close(currlinePtr->sock)){
	// 			fprintf(stderr,"discover: close() failed for %s\n",currlinePtr->ipaddr);
	// 			exit(1);
	// 		}
	// 	}
	// 	currlinePtr = currlinePtr->next;
	// }

	/* Disconnect from forwarding PCs */
	printf("Disconnecting from forwarding PCs...\n");
	currhostPtr = topdata.hosts;
	while(currhostPtr){
		// printf("Disconnecting from %s...\n",currhostPtr->addr);
		if(close(currhostPtr->sock)){
			fprintf(stderr,"setpath: close() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	// free(inet_addr);
	freeTop(topdata);

	return 0;
}


