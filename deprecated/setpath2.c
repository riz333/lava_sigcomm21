// setpath2.c -- created by Will Sussman on February 3, 2020

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "/home/riz3/willsLibrary/willslib.h"

int main(int argc, char **argv){

	/* Handle inputs */
	if(argc != 4){
		fprintf(stderr,"usage: setpath2 topfile PCport PCtimeout\n");
		fprintf(stderr,"example: setpath2 jan16top.txt 4013 0\n");
		// fprintf(stderr,"usage: setpath topfile PCport PCtimeout rxtxAddr rxtxPort rxtxTimeout\n");
		// fprintf(stderr,"example: setpath jan16top.txt 4013 0 172.29.220.218 4012 0\n");
		exit(1);
	}
	char *filename = argv[1];
	int PCport = atoi(argv[2]);
	float PCtimeout = atof(argv[3]);
	// char *rxtxAddr = argv[4];
	//int rxtxPort = atoi(argv[5]);
	// float rxtxTimeout = atof(argv[6]);
	// int nlinks = argc/4 - 1;
	// int startid[nlinks];
	// char startdir[nlinks];
	// int endid[nlinks];
	// char enddir[nlinks];
	// for(int i = 0; i < nlinks; i++){
	// 	startid[i] = atoi(argv[4 + 4*i]);
	// 	startdir[i] = argv[4 + 4*i + 1][0];
	// 	endid[i] = atoi(argv[4 + 4*i + 2]);
	// 	enddir[i] = argv[4 + 4*i + 3][0];
	// }

	/* Initialize data structure */
	FILE *topfile = fopen(filename,"r");
	if(!topfile){
		perror(filename);
		exit(1);
	}
	struct Topdata topdata;
	loadTop(topfile,&topdata);
	// printTop(stdout,topdata.list);
	fclose(topfile);
	// printTop(stdout,topdata.list);

	/* Connect to forwarding PCs */
	printf("Connecting to forwarding PCs...\n");
	struct Host *currhostPtr = topdata.hosts;
	while(currhostPtr){
		// printf("Connecting to %s...\n",currhostPtr->addr);
		if((currhostPtr->sock = openClient(currhostPtr->addr,PCport,PCtimeout)) == -1){
			fprintf(stderr,"setpath: openClient() failed for %s\n",currhostPtr->addr);
			exit(1);
		}
		currhostPtr = currhostPtr->next;
	}

	/* Generate paths from startid to endid */
	// printf("setpath: about to genPaths(&allpaths)\n");
	// struct Paths allpaths;
	// if(genPaths(&allpaths,topdata.list,startid,endid)){
	// 	fprintf(stderr,"setpath: genPaths() failed\n");
	// 	exit(1);
	// }
	// if(!allpaths.count){
	// 	fprintf(stderr,"setpath: no paths exist\n");
	// 	exit(1);
	// }
	// printf("setpath: about to printPaths(allpaths)\n");
	// printPaths(stdout,allpaths);

	/*
struct Link {
	int startid;
	char startdir;
	int endid;
	char enddir;
	struct Paths allpaths;
};

struct Links {
	int nlinks;
	struct Link *links;
};
	*/

	/* Connect to RxTx */
	//printf("Connecting to RxTx...\n");
	//int rxtxSock;
	// struct Host *currhostPtr = topdata.hosts;
	// while(currhostPtr){
		// printf("Connecting to %s...\n",currhostPtr->addr);
	//if((rxtxSock = openClient(rxtxAddr,rxtxPort,rxtxTimeout)) == -1){
	//	fprintf(stderr,"setpath: openClient() failed for %s\n",rxtxAddr);
	//	exit(1);
	//}
	// 	currhostPtr = currhostPtr->next;
	// }

	// struct Paths *allpaths = NULL;
	char request[3];
	struct Path minpath;
	// char *line = NULL;//, vdir;
	// size_t size = 0;
	struct Links links;
	int n, stat, idnum;//idnumV, idnumP;//, idMax, stat;
	// struct Max vBest, pBest;
	struct Report report, bestReport;
	struct Line *currlinePtr;
	// char dir;
	// int ntrials;
	// char triplet[3];
	// while(1){
		// printf("\nEnter links with format (startid startdir endid enddir)*\n");
		// printf("Enter v<dir><idnum> to query voltages with antenna <dir> (or all if -) on <idnum> (or all if 0)\n");
		// printf("\tExamples: v-0, vw23\n");
		// printf("Enter x to set off\n");
		// printf("Enter d<dir><ntrials> for discovery\n");
		// // printf("Enter w<cmd> to send <cmd> to RxTx\n");
		// printf("Enter l<idnum> to send listening command to <idnum> (or all if 0)\n");
		// printf("\tExamples: l0,l23\n");
		// printf("Enter CTRL+D to stop (without setting off)\n");

/*

0. Set everything off

1. The units are initially set on ‘listening mode’. The controller sends a command for them to be in this state (let this command be 'l - 0').
In this state, they are constantly measuring any voltage drops. When such a drop is detected, a message is sent to the central controller,
alerting it of the presence of traffic. This message is sent once by those units who detect a significant voltage drop.

2. The controller runs the ‘discovery’ process only on the units that reported a voltage drop. In this process, 10 measurements are taken for
each unit and the median voltage index for each is computed. The unit with the highest median voltage is chosen as the entry point.

3. The controller then runs the ‘discovery’ process on the entire network until the unit that reports the highest voltage index is other than
the one found in (2). This is an indication that the other endpoint is transmitting. The unit/direction reported after this process is the exit point.

4. The controller then enables the shortest path. At the entry and exit nodes, the command sent is of the form ('<in> <out> v'), where 'in' and 'out'
indicate the input and output antennas chosen for that unit, and 'v' indicates the unit that it should constantly report the voltage index to the controller.

5. The entry and exit units are constantly reporting the measured voltage to the controller. When BOTH units report a voltage index below a threshold,
the sessions is determined to be over and the controller turns off the path.

6. Step 1 restarts

*/

	printf("\nPHASE 0\n");

	printf("Generating off commands...\n");
	request[0] = '-'; request[1] = '-'; request[2] = '0';
	if(prepGivenCmds(topdata.list,request)){
		fprintf(stderr,"setpath2: prepOffCmds() failed\n");
		exit(1);
	}

	printf("Sending commands to forwading PCs...\n");
	if(sendAllCmds(topdata.list)){
		fprintf(stderr,"setpath2: sendAllCmds() failed\n");
		exit(1);
	}

	printf("\nPHASE 1\n");

	printf("Generating listen commands...\n");
	request[0] = '-'; request[1] = '-'; request[2] = 'l';
	if(prepGivenCmds(topdata.list,request)){
		fprintf(stderr,"setpath2: prepGivenCmds() failed\n");
		exit(1);
	}

	printf("Sending commands to forwading PCs...\n");
	if(sendAllCmds(topdata.list)){
		fprintf(stderr,"setpath2: sendAllCmds() failed\n");
		exit(1);
	}

	printf("Scanning for alerts...\n");
	// scan() will return 0 if an alert is detected, otherwise 1 (error)
	if(scan(topdata.list)){
		fprintf(stderr,"setpath2: scan() failed\n");
		exit(1);
	}

	printf("\nPHASE 2\n");

	char dir = 'e';
	int ntrials = 10;

	currhostPtr = topdata.hosts;
	while(currhostPtr){
		printf("Requesting d report from host %s...\n",currhostPtr->addr);
		request[0] = dir; request[1] = '-'; request[2] = 'd';
		if(sendQuad(currhostPtr->sock,ntrials,request)){
			fprintf(stderr,"setpath2: sendQuad() failed\n");
			exit(1);
		}

		currhostPtr = currhostPtr->next;
	}

	currhostPtr = topdata.hosts;
	while(currhostPtr){
		printf("Getting d report from host %s...\n",currhostPtr->addr);
		stat = getReport(currhostPtr->sock,&report);
		if(!stat){
			fprintf(stderr,"setpath2: getReport() failed to read from %s\n",currhostPtr->addr);
			exit(1);
		} else if(stat == -1){
			fprintf(stderr,"setpath2: getReport() failed for %s\n",currhostPtr->addr);
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

	currlinePtr = lineLookup(topdata.list,bestReport.idnum);

	printf("Requesting final d report from host %s...\n",currlinePtr->host->addr);
	request[0] = '-'; request[1] = (char)bestReport.idnum; request[2] = 'd';
	if(sendQuad(currlinePtr->host->sock,ntrials,request)){
		fprintf(stderr,"setpath2: sendQuad() failed\n");
		exit(1);
	}

	printf("Getting final d report from host %s...\n",currlinePtr->host->addr);
	stat = getReport(currlinePtr->host->sock,&bestReport);
	if(!stat){
		fprintf(stderr,"setpath2: getReport() failed to read from %s\n",currlinePtr->host->addr);
		exit(1);
	} else if(stat == -1){
		fprintf(stderr,"setpath2: getReport() failed for %s\n",currlinePtr->host->addr);
		exit(1);
	}

	printf("Report: #%d(%c,%f)\n",bestReport.idnum,bestReport.data.dir,bestReport.data.val);



		// if((n = getline(&line,&size,stdin)) == EOF){
		// 	break;
		// } else if(n == -1){
		// 	fprintf(stderr,"setpath: getline() failed\n");
		// 	exit(1);
		// }

		if(line[0] == 'x'){
			printf("Setting off\n");
			if(prepOffCmds(topdata.list)){
				fprintf(stderr,"setpath: prepOffCmds() failed\n");
				exit(1);
			}
		} else if(line[0] == 'l'){
			idnum = atoi(&line[1]);
			request[0] = '-'; request[1] = '-'; request[2] = 'l';
			if(!idnum){
				printf("Generating listening commands for all...\n",idnum);
				prepGivenCmds(topdata.list, request);
				// will be sent outside of if statement
			} else {
				printf("Sending listening command to %d...\n",idnum);
				currlinePtr = lineLookup(topdata.list,idnum);
				if(sendQuad(currhostPtr->sock,idnum,request)){
					fprintf(stderr,"getReport(): sendQuad() failed\n");
					exit(1);
				}
				free(line);
				line = NULL;
				size = 0;
				continue;
			}

		// } else if(line[0] == 'w'){

			//printf("Sending the following to RxTx: %s",&line[1]);

			//stat = write(rxtxSock,&line[1],strlen(&line[1]));
			//if(!stat){
			//	fprintf(stderr,"setpath: write() returned without writing\n");
			//	exit(1);
			//} else if(stat < strlen(&line[1])){
			//	fprintf(stderr,"setpath: write() only wrote %d/%ld\n",stat,strlen(&line[1]));
			//	exit(1);
			//} else if(stat == -1){
			//	fprintf(stderr,"setpath: write() failed\n");
			//	exit(1);
			//}

			// free(line);
			// line = NULL;
			// size = 0;
			// continue;
		} else if(line[0] == 'd'){

			// d e 10
			// 0 1 2

			dir = line[1];
			ntrials = atoi(&line[2]);

			currhostPtr = topdata.hosts;
			while(currhostPtr){
				printf("Requesting d report from host %s...\n",currhostPtr->addr);
				request[0] = dir; request[1] = '-'; request[2] = 'd';
				if(sendQuad(currhostPtr->sock,ntrials,request)){
					fprintf(stderr,"getReport(): sendQuad() failed\n");
					exit(1);
				}
				// stat = getReport(currhostPtr->sock,ntrials,dir,'-','d',&report);
				// printf("Done\n");
				// if(!stat){
				// 	fprintf(stderr,"setpath: getReport() failed to read from %s\n",currhostPtr->addr);
				// 	exit(1);
				// } else if(stat == -1){
				// 	fprintf(stderr,"setpath: getReport() failed for %s\n",currhostPtr->addr);
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
					fprintf(stderr,"setpath: getReport() failed to read from %s\n",currhostPtr->addr);
					exit(1);
				} else if(stat == -1){
					fprintf(stderr,"setpath: getReport() failed for %s\n",currhostPtr->addr);
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

			currlinePtr = lineLookup(topdata.list,bestReport.idnum);

			printf("Requesting final d report from host %s...\n",currlinePtr->host->addr);
			request[0] = '-'; request[1] = (char)bestReport.idnum; request[2] = 'd';
			if(sendQuad(currlinePtr->host->sock,ntrials,request)){
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
			// request[0] = '-'; request[1] = (char)bestReport.idnum; request[2] = 'd';
			// if(sendQuad(currlinePtr->host->sock,ntrials,request)){
			// 	fprintf(stderr,"getReport(): sendQuad() failed\n");
			// 	exit(1);
			// }
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

			free(line);
			line = NULL;
			size = 0;
			continue;

		} else if(line[0] == 'v'){

			idnum = atoi(&line[2]);

			if(idnum){
				currlinePtr = lineLookup(topdata.list,idnum);


				printf("Requesting v report from host %s...\n",currlinePtr->host->addr);
				request[0] = line[1]; request[1] = '-'; request[2] = 'v';
				if(sendQuad(currlinePtr->host->sock,idnum,request)){
					fprintf(stderr,"getReport(): sendQuad() failed\n");
					exit(1);
				}
				// stat = getReport(currlinePtr->host->sock,idnum,line[1],'-','v',&bestReport);
				// // printf("Done\n");
				// if(!stat){
				// 	fprintf(stderr,"setpath: getReport() failed to read from %s\n",currlinePtr->host->addr);
				// 	exit(1);
				// } else if(stat == -1){
				// 	fprintf(stderr,"setpath: getReport() failed for %s\n",currlinePtr->host->addr);
				// 	exit(1);
				// }

				printf("Getting v report from host %s...\n",currlinePtr->host->addr);
				// request[0] = line[1]; request[1] = '-'; request[2] = 'v';
				// if(sendQuad(currlinePtr->host->sock,idnum,request)){
				// 	fprintf(stderr,"getReport(): sendQuad() failed\n");
				// 	exit(1);
				// }
				stat = getReport(currlinePtr->host->sock,&bestReport);
				// printf("Done\n");
				if(!stat){
					fprintf(stderr,"setpath: getReport() failed to read from %s\n",currlinePtr->host->addr);
					exit(1);
				} else if(stat == -1){
					fprintf(stderr,"setpath: getReport() failed for %s\n",currlinePtr->host->addr);
					exit(1);
				}

			} else {
				currhostPtr = topdata.hosts;
				while(currhostPtr){
					printf("Requesting v report from host %s...\n",currhostPtr->addr);
					request[0] = line[1]; request[1] = '-'; request[2] = 'v';
					if(sendQuad(currhostPtr->sock,0,request)){
						fprintf(stderr,"getReport(): sendQuad() failed\n");
						exit(1);
					}
					// stat = getReport(currhostPtr->sock,0,line[1],'-','v',&report);
					// // printf("Done\n");
					// if(!stat){
					// 	fprintf(stderr,"setpath: getReport() failed to read from %s\n",currhostPtr->addr);
					// 	exit(1);
					// } else if(stat == -1){
					// 	fprintf(stderr,"setpath: getReport() failed for %s\n",currhostPtr->addr);
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
					printf("Getting v report from host %s...\n",currhostPtr->addr);
					// request[0] = line[1]; request[1] = '-'; request[2] = 'v';
					// if(sendQuad(currhostPtr->sock,0,request)){
					// 	fprintf(stderr,"getReport(): sendQuad() failed\n");
					// 	exit(1);
					// }
					stat = getReport(currhostPtr->sock,&report);
					// printf("Done\n");
					if(!stat){
						fprintf(stderr,"setpath: getReport() failed to read from %s\n",currhostPtr->addr);
						exit(1);
					} else if(stat == -1){
						fprintf(stderr,"setpath: getReport() failed for %s\n",currhostPtr->addr);
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
			}

			printf("Report: #%d(%c,%f)\n",bestReport.idnum,bestReport.data.dir,bestReport.data.val);

			free(line);
			line = NULL;
			size = 0;
			continue;
		} else {
			if(parseLinks(&links,line)){
				fprintf(stderr,"setpath: parseLinks() failed\n");
				exit(1);
			}

			// struct Paths allpaths[nlinks];
			// struct Path minpath;
			for(int i = 0; i < links.nlinks; i++){
				if(genPaths(&links.linkset[i].allpaths,topdata.list,links.linkset[i].startid,links.linkset[i].endid,true)){ // true for disjoint
					fprintf(stderr,"setpath: genPaths() failed for %d->%d\n",links.linkset[i].startid,links.linkset[i].endid);
					exit(1);
				}
				if(!links.linkset[i].allpaths.count){
					fprintf(stderr,"setpath: no such paths exist for %d->%d\n",links.linkset[i].startid,links.linkset[i].endid);
					exit(1);
				}
				// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
				// printPaths(stdout,links.linkset[i].allpaths);
				
				minpath = links.linkset[i].allpaths.paths[0];
				for(int j = 1; j < links.linkset[i].allpaths.count; j++){
					if(links.linkset[i].allpaths.paths[j].len < minpath.len){
						minpath = links.linkset[i].allpaths.paths[j];
					}
				}
				printf("Minpath from %d->%d: ",links.linkset[i].startid,links.linkset[i].endid);
				printPath(stdout,minpath);
				for(int j = 0; j < minpath.len; j++){
					minpath.nodes[j]->used = true;
				}

				if(prepCmds(topdata.list,links.linkset[i].startdir,minpath,links.linkset[i].enddir)){
					fprintf(stderr,"setpath: prepCmds() failed\n");
					exit(1);
				}

				freePaths(links.linkset[i].allpaths);
			}
			free(links.linkset);
		}

		currlinePtr = topdata.list;
		while(currlinePtr){
			currlinePtr->used = false;
			currlinePtr = currlinePtr->next;
		}

		/* Send commands to forwarding PCs */
		printf("Sending commands to forwading PCs...\n");
		if(sendAllCmds(topdata.list)){
			fprintf(stderr,"setpath: sendAllCmds() failed\n");
			exit(1);
		}

		/* Select shortest path */
		// struct Path minpath = allpaths.paths[0];
		// for(int i = 1; i < allpaths.count; i++){
		// 	if(allpaths.paths[i].len < minpath.len){
		// 		minpath = allpaths.paths[i];
		// 	}
		// }
		// printf("setpath: about to printPath(minpath)\n");
		// printf("Shortest path: ");
		// printPath(stdout,minpath);

		/* Prepare commands for each Arduino */
		// printf("setpath: about to sendAllCmds()\n");
		// if(prepCmds(topdata.list,startdir,minpath,enddir)){
		// 	fprintf(stderr,"setpath: prepCmds() failed\n");
		// 	exit(1);
		// }
		// printf("setpath: about to freePaths(allpaths)\n");
		// freePaths(allpaths);

		// free(line);
		// line = NULL;
		// size = 0;
	// }
	// free(line);

	/* Testing */
	// printf("Press enter to send\n");
	// getchar();
	// char *msg = "Test";
	// int len = 4, nsent;
	// currhostPtr = topdata.hosts;
	// while(currhostPtr){
	// 	printf("About to send \"%s\"\n",msg);
	// 	nsent = send(currhostPtr->sock,msg,len,0);
	// 	if(nsent == -1){
	// 		perror(currhostPtr->addr);
	// 	} else {
	// 		printf("nsent %d to %s\n",nsent,currhostPtr->addr);
	// 	}
	// 	currhostPtr = currhostPtr->next;
	// }
	// printf("Press enter to send\n");
	// getchar();
	// char *msg = "Test";
	// int len = 4, nsent;
	// currhostPtr = topdata.hosts;
	// while(currhostPtr){
	// 	printf("About to send \"%s\"\n",msg);
	// 	nsent = write(currhostPtr->sock,msg,len);
	// 	if(nsent == -1){
	// 		perror(currhostPtr->addr);
	// 	} else {
	// 		printf("nsent %d to %s\n",nsent,currhostPtr->addr);
	// 	}
	// 	currhostPtr = currhostPtr->next;
	// }

	// printf("Press enter to finish\n");
	// getchar();

	//printf("Disconnecting from RxTx...\n");
	//if(close(rxtxSock)){
	//	fprintf(stderr,"setpath: close() failed for %s\n",rxtxAddr);
	//	exit(1);
	//}

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

	// printf("setpath: about to freeTop(topdata)\n");
	freeTop(topdata);
	return 0;
}
