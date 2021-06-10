// willslib.c -- created by Will Sussman on December 26, 2019

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include "willslib.h"
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

/* Initializes the main data structure */
int loadTop(FILE *stream, struct Topdata *topdata){
	topdata->inet_addr = NULL;
	topdata->localSocks = NULL;
	topdata->list = NULL;
	topdata->hosts = NULL;
	topdata->nhosts = 0;
	topdata->nlocal = 0;
	topdata->maxID = 0;
	// topdata->numEnds = 0;
	// topdata->nlines = 0;
	// topdata->stop = false;
	char *line = NULL;
	size_t size = 0;
	while(getline(&line,&size,stream) != EOF){
		if(parseLine(line,topdata)){
			fprintf(stderr,"loadTop(): parseLine() failed\n");
			return 1;
		}
		// topdata->nlines++;
		free(line);
		line = NULL;
		size = 0;
	}
	if(line != NULL){
		free(line);
	}

	topdata->hostSocks = malloc(sizeof(*topdata->hostSocks) * topdata->nhosts);
	if(!topdata->hostSocks){
		fprintf(stderr,"loadTop(): malloc() failed\n");
		return 1;
	}

	struct Line *currlinePtr = topdata->list;
	while(currlinePtr){
		if(currlinePtr->idnum > topdata->maxID){
			topdata->maxID = currlinePtr->idnum;
		}
		currlinePtr = currlinePtr->next;
	}

	// printf("maxID is %d\n",topdata->maxID);
	topdata->near = malloc(sizeof(*topdata->near) * (topdata->maxID + 1));
	for(int i = 0; i <= topdata->maxID; i++){
		topdata->near[i] = malloc(sizeof(*topdata->near[i]) * (topdata->maxID + 1));
		// memset(topdata->near[i],0,sizeof(*topdata->near[i]));
	}

	// initialize near table to all 0
	for(int i = 0; i < topdata->maxID + 1; i++){
		for(int j = 0; j < topdata->maxID + 1; j++){
			// printf("(%d,%d)=%d\t",i,j,topdata->near[i][j]);
			topdata->near[i][j] = 0;
		}
		// printf("\n");
	}

	currlinePtr = topdata->list;
	while(currlinePtr){
		for(int i = 0; i < currlinePtr->neighbors.count; i++){ // loop over 1-hop neighbors
			struct Line *neighborPtr = currlinePtr->neighbors.lines[i];
			// printf("Setting (%d,%d) to 1\n",currlinePtr->idnum,neighborPtr->idnum);


			topdata->near[currlinePtr->idnum][neighborPtr->idnum] = 1;
			for(int j = 0; j < neighborPtr->neighbors.count; j++){ // loop over 2-hop neighbors
				struct Line *neighborPtr2 = neighborPtr->neighbors.lines[j];
				// printf("Setting (%d,%d) to 1\n",currlinePtr->idnum,neighborPtr2->idnum);
				topdata->near[currlinePtr->idnum][neighborPtr2->idnum] = 1;
			}
		}
		currlinePtr = currlinePtr->next;
	}

	// printf("Dumping near table\n");
	// for(int i = 0; i < topdata->maxID + 1; i++){
	// 	for(int j = 0; j < topdata->maxID + 1; j++){
	// 		printf("(%d,%d)=%d\t",i,j,topdata->near[i][j]);
	// 	}
	// 	printf("\n");
	// }

	return 0;
}

/* Dumps the main data structure to the given stream (e.g., stdout) */
int printTop(FILE *stream, struct Line *datalist){
	struct Line *currlinePtr = datalist;
	while(currlinePtr){
		if(currlinePtr->set){
			printLine(stream,currlinePtr);
		}
		currlinePtr = currlinePtr->next;
	}
	return 0;
}

/* Frees the main data structure, including linked memory */
int freeTop(struct Topdata topdata){
	struct Line *currlinePtr = topdata.list, *nextlinePtr;
	while(currlinePtr){
		nextlinePtr = currlinePtr->next;
		freeLine(currlinePtr);
		currlinePtr = nextlinePtr;
	}
	struct Host *currhostPtr = topdata.hosts, *nexthostPtr;
	while(currhostPtr){
		nexthostPtr = currhostPtr->next;
		free(currhostPtr->addr);
		free(currhostPtr);
		currhostPtr = nexthostPtr;
	}
	if(topdata.inet_addr){
		free(topdata.inet_addr);
	}
	free(topdata.hostSocks);
	if(topdata.localSocks){
		free(topdata.localSocks);
	}

	for(int i = 0; i < topdata.maxID; i++){
		free(topdata.near[i]);
	}
	free(topdata.near);

	return 0;
}

/* Adds a line to the abstract topology file without editing neighbors */
int topAdd(struct Line **topdata, struct Line *linePtr){
	if(!*topdata){
		*topdata = linePtr;
		linePtr->next = NULL;
		return 0;
	}
	struct Line *currlinePtr = *topdata, *prevlinePtr = NULL;
	while(linePtr->idnum > currlinePtr->idnum){
		prevlinePtr = currlinePtr;
		if(!(currlinePtr = currlinePtr->next)){
			break;
		}
	}
	if(prevlinePtr){
		prevlinePtr->next = linePtr;
	} else {
		*topdata = linePtr;
	}
	linePtr->next = currlinePtr;
	return 0;
}

/* Deletes a line from the abstract topology file without editing neighbors */
int topDelete(struct Line **topdata, struct Line *linePtr){
	if(!*topdata){
		fprintf(stderr,"topDelete(): cannot delete from empty topdata\n");
		return 1;
	}
	struct Line *currlinePtr = *topdata, *prevlinePtr = NULL;
	while(linePtr->idnum != currlinePtr->idnum){
		prevlinePtr = currlinePtr;
		if(!(currlinePtr = currlinePtr->next)){
			fprintf(stderr,"topDelete(): cannot delete nonexistent line\n");
			return 1;
		}
	}
	if(prevlinePtr){
		prevlinePtr->next = currlinePtr->next;
	}
	freeLine(currlinePtr);
	return 0;
}

/* Adds a line to the abstract topology file and edits neighbors */
int topInsert(struct Line **topdata, struct Line *linePtr){
	if(!*topdata){
		*topdata = linePtr;
		linePtr->next = NULL;
		return 0;
	}
	struct Line *currlinePtr = *topdata, *prevlinePtr = NULL;
	char dir;
	while(linePtr->idnum > currlinePtr->idnum){
		if(isNeighbor(currlinePtr->idnum,linePtr->neighbors,&dir)){
			addNeighbor(linePtr,&currlinePtr->neighbors,oppDir(dir));
		}
		prevlinePtr = currlinePtr;
		if(!(currlinePtr = currlinePtr->next)){
			break;
		}
	}
	if(prevlinePtr){
		prevlinePtr->next = linePtr;
	}
	linePtr->next = currlinePtr;
	while(currlinePtr){
		if(isNeighbor(currlinePtr->idnum,linePtr->neighbors,&dir)){
			addNeighbor(linePtr,&currlinePtr->neighbors,oppDir(dir));
		}
		currlinePtr = currlinePtr->next;
	}
	return 0;
}

/* Deletes a line from the abstract topology file and edits neighbors */
int topRemove(struct Line **topdata, struct Line *linePtr){
	if(!*topdata){
		fprintf(stderr,"topRemove(): cannot remove from empty topdata\n");
		return 1;
	}
	struct Line *currlinePtr = *topdata, *prevlinePtr = NULL;
	while(linePtr->idnum != currlinePtr->idnum){
		if(isNeighbor(currlinePtr->idnum,linePtr->neighbors,NULL)){
			deleteNeighbor(linePtr->idnum,&currlinePtr->neighbors);
		}
		prevlinePtr = currlinePtr;
		if(!(currlinePtr = currlinePtr->next)){
			fprintf(stderr,"topRemove(): cannot remove nonexistent line\n");
			return 1;
		}
	}
	if(prevlinePtr){
		prevlinePtr->next = currlinePtr->next;
	}
	prevlinePtr = currlinePtr;
	while(currlinePtr){
		if(isNeighbor(currlinePtr->idnum,linePtr->neighbors,NULL)){
			deleteNeighbor(linePtr->idnum,&currlinePtr->neighbors);
		}
	}
	freeLine(prevlinePtr);
	return 0;
}

/* Converts line from topology file to abstract line in main data structure */
int parseLine(char *line, struct Topdata *topdata){
	int idnum = atoi(line);
	struct Line *linePtr = lineLookup(topdata->list,idnum), *neighborLine;
	bool newblock = !linePtr;
	if(newblock){
		linePtr = malloc(sizeof(*linePtr));
		if(!linePtr){
			fprintf(stderr,"parseLine(): malloc() failed\n");
			return 1;
		}
		linePtr->idnum = idnum;
	}

	int i = 0;
	while(line[i++] != '\t');
	int j = i;
	while(line[j++] != '\t');
	// linePtr->host->addr = malloc(sizeof(*linePtr->host->addr) * (j - i));
	// strncpy(linePtr->host->addr,line + i,j - i - 1);
	// linePtr->host->addr[j-i-1] = '\0';

	bool newhost = true;
	struct Host *currhostPtr = topdata->hosts, *prevhostPtr = NULL;
	while(currhostPtr){
		if(!strncmp(line + i,currhostPtr->addr,j - i - 1)){
			newhost = false;
			linePtr->host = currhostPtr;
			break;
		}
		prevhostPtr = currhostPtr;
		currhostPtr = currhostPtr->next;
	}
	if(newhost){
		topdata->nhosts++;
		// topdata->hosts = realloc(topdata->hosts,sizeof(*topdata->hosts) * ++topdata->nhosts);
		// linePtr->host = &topdata->hosts[topdata->nhosts - 1];
		// printf("newhost\n");
		struct Host *newhostPtr = malloc(sizeof(*newhostPtr));

		newhostPtr->addr = malloc(sizeof(*linePtr->host->addr) * (j - i));
		strncpy(newhostPtr->addr,line + i,j - i - 1);
		newhostPtr->addr[j-i-1] = '\0';

		linePtr->host = newhostPtr;

		// if(currhostPtr){
		// 	printf("curr is %s\n",currhostPtr->addr);
		// } else {
		// 	printf("curr is null\n");
		// }
		if(prevhostPtr){
			// printf("prev is %s\n",prevhostPtr->addr);
			prevhostPtr->next = newhostPtr;
		} else {
			// printf("prev is null\n");
			topdata->hosts = newhostPtr;
		}
		newhostPtr->next = currhostPtr;


		// printf("NOW:\n");
		// if(prevhostPtr){
		// 	printf("prev is %s\n",prevhostPtr->addr);
		// } else {
		// 	printf("prev is null\n");
		// }
		// printf("new is %s\n",newhostPtr->addr);
		// if(currhostPtr){
		// 	printf("curr is %s\n",currhostPtr->addr);
		// } else {
		// 	printf("curr is null\n");
		// }
		// struct Host *ptr = topdata->hosts;
		// while(ptr){
		// 	printf("currblock is %s\n",ptr->addr);
		// 	if(!ptr->next){
		// 		printf("nextblock is null\n");
		// 	}
		// 	ptr = ptr->next;
		// }
	}

	// currhostPtr = topdata->hosts;
	// int n = 1;
	// while(currhostPtr){
	// 	printf("host %d: %s\n",n++,currhostPtr->addr);
	// }

	linePtr->seen = false;
	linePtr->used = false;
	linePtr->queried = false;
	linePtr->clock = 0;
	linePtr->triplet[0] = 'e';
	linePtr->triplet[1] = '-';
	linePtr->triplet[2] = '0';
	linePtr->historyIndex = 0;
	linePtr->tableFull = false;
	linePtr->isAnEnd = false;

	i = j;
	while(line[i++] != '\t');
	linePtr->ipaddr = malloc(sizeof(*linePtr->ipaddr) * (i - j));
	strncpy(linePtr->ipaddr,line + j,i - j - 1);
	linePtr->ipaddr[i-j-1] = '\0';

	j = i;
	while(line[i++] != '\t');
	linePtr->ants.num = i - j - 1;
	linePtr->ants.array = malloc(sizeof(*linePtr->ants.array) * linePtr->ants.num);
	strncpy(linePtr->ants.array,line + j,linePtr->ants.num);

	j = i;
	while(line[i++] != '\t');
	linePtr->neighbors.count = i - j - 1;
	// printf("count is: %d\n",linePtr->neighbors.count);
	linePtr->neighbors.idnums = malloc(sizeof(*linePtr->neighbors.idnums) * linePtr->neighbors.count);
	linePtr->neighbors.dirs = malloc(sizeof(*linePtr->neighbors.dirs) * linePtr->neighbors.count);
	linePtr->neighbors.lines = malloc(sizeof(*linePtr->neighbors.lines) * linePtr->neighbors.count);
	strncpy(linePtr->neighbors.dirs,line + j,linePtr->neighbors.count);



	linePtr->nonpath.num = linePtr->ants.num - 1;
	if(linePtr->nonpath.num < 1){
		fprintf(stderr,"parseLine(): not enough antennas on #%d\n",linePtr->idnum);
		return 1;
	}
	linePtr->nonpath.array = malloc(sizeof(*linePtr->nonpath.array) * linePtr->nonpath.num);
	// leave empty for now

	// int numCommas = 0;
	// for(j = i; line[j] != '\t'; j++){
	// 	if(line[j] == ','){
	// 		numCommas++;
	// 	}
	// }

	// if(j == i){
	// 	linePtr->neighbors.count = 0;
	// 	linePtr->neighbors.idnums = NULL;
	// 	linePtr->neighbors.lines = NULL;
	// 	linePtr->neighbors.dirs = NULL;
	// 	linePtr->set = true;
	// 	if(newblock){
	// 		topAdd(&topdata->list,linePtr);
	// 	}
	// 	return 0;
	// }

	// linePtr->neighbors.count = numCommas + 1;
	// linePtr->neighbors.idnums = malloc(sizeof(*linePtr->neighbors.idnums) * linePtr->neighbors.count);
	// linePtr->neighbors.dirs = malloc(sizeof(*linePtr->neighbors.dirs) * linePtr->neighbors.count);
	// linePtr->neighbors.lines = malloc(sizeof(*linePtr->neighbors.lines) * linePtr->neighbors.count);
	char *ptr = line + i - 1;
	// j--;
	for(i = 0; i < linePtr->neighbors.count; i++){
		linePtr->neighbors.idnums[i] = strtol(ptr + 1,&ptr,10); // Neat trick!
		// linePtr->neighbors.dirs[i] = line[j += 2];

		neighborLine = lineLookup(topdata->list,linePtr->neighbors.idnums[i]);
		if(neighborLine){
			linePtr->neighbors.lines[i] = neighborLine;
		} else {
			linePtr->neighbors.lines[i] = malloc(sizeof(*linePtr->neighbors.lines[i]));
			linePtr->neighbors.lines[i]->idnum = linePtr->neighbors.idnums[i];
			linePtr->neighbors.lines[i]->set = false;
			topAdd(&topdata->list,linePtr->neighbors.lines[i]);
		}

	}
	linePtr->set = true;
	if(newblock){
		topAdd(&topdata->list,linePtr);
	}
	return 0;
}

/* Dumps an abstract line form the main data structure to the given stream (e.g., stdout) */
int printLine(FILE *stream, struct Line *linePtr){

	fprintf(stream,"%d\t%s\t%s\t",linePtr->idnum,linePtr->host->addr,linePtr->ipaddr);

	for(int i = 0; i < linePtr->ants.num; i++){
		fprintf(stream,"%c",linePtr->ants.array[i]);
	}
	fprintf(stream,"\t");

	if(linePtr->neighbors.count == 0){
		fprintf(stream,"\t\n");
		return 0;
	}

	for(int i = 0; i < linePtr->neighbors.count; i++){
		fprintf(stream,"%c",linePtr->neighbors.dirs[i]);
	}
	fprintf(stream,"\t");

	for(int i = 0; i < linePtr->neighbors.count - 1; i++){
		fprintf(stream,"%d,",linePtr->neighbors.idnums[i]);
	}
	fprintf(stream,"%d\n",linePtr->neighbors.idnums[linePtr->neighbors.count - 1]);
	return 0;
}

/*
struct Line {
	int idnum; // ID number of the node
	struct Host *host; // pointer to the Host that controls the node
	char *ipaddr; // IP address of the node
	struct Neighbors neighbors; // neighbors of the node
	struct Line *next; // pointer to next node in linked list
	bool seen; // used for path generation; whether the node is already in the path
	bool used; // used for path generation; whether the node is already in a chosen path
	bool set; // used for data structure initialization; whether the block has been initialized
	int sock; // socket of the node
	char triplet[3]; // used for sending commands; holds the 3-byte command
	float voltage;
	float powerIndex;
	struct Antennas ants;
};
*/

/* Frees an abstract line from the main data structure, including linked memory */
int freeLine(struct Line *linePtr){
	// free(linePtr->host->addr);
	free(linePtr->ipaddr);
	free(linePtr->neighbors.idnums);
	free(linePtr->neighbors.dirs);
	free(linePtr->nonpath.array);
	free(linePtr->neighbors.lines);
	// free(linePtr->triplet);
	free(linePtr->ants.array);
	free(linePtr);
	return 0;
}

/* Takes in a node and another node's neighbors. If the node is among
   the neighbors, its direction from the other node is set, and 1 is
   returned. Otherwise 0 is returned. */
int isNeighbor(int idnum, struct Neighbors neighbors, char *dir){
	for(int i = 0; i < neighbors.count; i++){
		if(neighbors.idnums[i] == idnum){
			if(dir != NULL){
				*dir = neighbors.dirs[i];
			}
			return 1;
		}
	}
	return 0;
}

/* Edits the set of neighbors */
int addNeighbor(struct Line *linePtr, struct Neighbors *neighborsPtr, char dir){
	int *newIdnums = malloc(sizeof(*newIdnums) * (neighborsPtr->count + 1));
	struct Line **newLines = malloc(sizeof(*newLines) * (neighborsPtr->count + 1));
	char *newDirs = malloc(sizeof(*newDirs) * (neighborsPtr->count + 1));
	int i;
	for(i = 0; i < neighborsPtr->count; i++){
		if(neighborsPtr->idnums[i] > linePtr->idnum){
			break;
		}
		newIdnums[i] = neighborsPtr->idnums[i];
		newLines[i] = neighborsPtr->lines[i];
		newDirs[i] = neighborsPtr->dirs[i];
	}
	newIdnums[i] = linePtr->idnum;
	newLines[i] = linePtr;
	newDirs[i] = dir;
	while(i < neighborsPtr->count){
		newIdnums[i+1] = neighborsPtr->idnums[i];
		newLines[i+1] = neighborsPtr->lines[i];
		newDirs[i+1] = neighborsPtr->dirs[i];
		i++;
	}
	neighborsPtr->count++;
	free(neighborsPtr->idnums);
	neighborsPtr->idnums = newIdnums;
	free(neighborsPtr->lines);
	neighborsPtr->lines = newLines;
	free(neighborsPtr->dirs);
	neighborsPtr->dirs = newDirs;
	return 0;
}

/* Edits the set of neighbors */
int deleteNeighbor(int idnum, struct Neighbors *neighborsPtr){
	int *newIdnums = malloc(sizeof(*newIdnums) * (neighborsPtr->count - 1));
	struct Line **newLines = malloc(sizeof(*newLines) * (neighborsPtr->count - 1));
	char *newDirs = malloc(sizeof(*newDirs) * (neighborsPtr->count - 1));
	int i;
	for(i = 0; i < neighborsPtr->count; i++){
		if(neighborsPtr->idnums[i] == idnum){
			break;
		}
		newIdnums[i] = neighborsPtr->idnums[i];
		newLines[i] = neighborsPtr->lines[i];
		newDirs[i] = neighborsPtr->dirs[i];
	}
	while(i < neighborsPtr->count){
		newIdnums[i] = neighborsPtr->idnums[i+1];
		newLines[i] = neighborsPtr->lines[i+1];
		newDirs[i] = neighborsPtr->dirs[i+1];
		i++;
	}
	neighborsPtr->count--;
	free(neighborsPtr->idnums);
	neighborsPtr->idnums = newIdnums;
	free(neighborsPtr->lines);
	neighborsPtr->lines = newLines;
	free(neighborsPtr->dirs);
	neighborsPtr->dirs = newDirs;
	return 0;
}

/* Returns the cardinal-opposite direction character, or - if not given n/e/s/w */
char oppDir(char dir){
	if(dir == 'n'){
		return 's';
	} else if(dir == 'e'){
		return 'w';
	} else if(dir == 's'){
		return 'n';
	} else if(dir == 'w'){
		return 'e';
	} else {
		fprintf(stderr,"oppDir(): invalid direction, defaulting to '-'\n");
		return '-';
	}
}

/* Returns handle to abstract line with ID number idnum */
struct Line *lineLookup(struct Line *datalist, int idnum){
	struct Line *currlinePtr = datalist;
	while(currlinePtr && idnum != currlinePtr->idnum){
		currlinePtr = currlinePtr->next;
	}
	return currlinePtr;
}

/* Recursively generates a list of paths, allpaths, from startid to endid, optionally disjoint */
int genPaths(struct Paths *allpaths, struct Line *datalist, int startid, int endid, bool disjoint){
	struct Line *startline = lineLookup(datalist,startid);
	if(!startline){
		fprintf(stderr,"genPaths(): startid does not exist\n");
		return 1;
	}
	
	struct Path path;
	path.len = 0;
	path.nodes = NULL;
	struct Paths pathset;
	pathset.count = 0;
	pathset.paths = NULL;

	genPathsAux(startline,&path,&pathset,endid,disjoint);
	*allpaths = pathset;
	return 0;
}

/* Helper function for genPaths, does the recursive path generation */
int genPathsAux(struct Line *currlinePtr, struct Path *path, struct Paths *pathset, int endid, bool disjoint){
	pathPush(path,currlinePtr);
	if(currlinePtr->idnum == endid){
		savePath(path,pathset);
		pathPop(path);
		return 0;
	}
	currlinePtr->seen = true;
	for(int i = 0; i < currlinePtr->neighbors.count; i++){
		if(!currlinePtr->neighbors.lines[i]->seen && (disjoint ? !currlinePtr->neighbors.lines[i]->used : true)){
			genPathsAux(currlinePtr->neighbors.lines[i],path,pathset,endid,disjoint);
		}
	}
	pathPop(path);
	currlinePtr->seen = false;
	return 0;
}

/*
int openAllComms(struct Line *topdata, int port, float timeout){
	struct Line *currlinePtr = topdata;
	// struct sockaddr_in server;
	// server.sin_family = AF_INET;
	// server.sin_port = htons(port);
	// int nfds;
	// fd_set fdset;
	// struct timeval tv;
	// long long usecs = timeout * 1000000;
	// tv.tv_sec = usecs / 1000000;
	// tv.tv_usec = usecs % 1000000;
	// int optval;
	// socklen_t optlen = sizeof(optval);
	while(currlinePtr){
		if((currlinePtr->sock = openComms(currlinePtr->ipaddr,port,timeout)) == -1){
			fprintf(stderr,"openAllComms(): openComms() failed for %s\n",currlinePtr->ipaddr);
			return 1;
		}
		// if((currlinePtr->sock = socket(AF_INET,SOCK_STREAM,0)) == -1){
		// 	fprintf(stderr,"openAllComms(): socket() failed at line %d\n",currlinePtr->idnum);
		// 	return 1;
		// }
		// if(fcntl(currlinePtr->sock,F_SETFL,O_NONBLOCK) == -1){
		// 	fprintf(stderr,"openAllComms(): fcntl() failed at line %d\n",currlinePtr->idnum);
		// 	return 1;
		// }
		// server.sin_addr.s_addr = inet_addr(currlinePtr->ipaddr);
		// // if(inet_pton(AF_INET,currlinePtr->ipaddr,&server.sin_addr) != 1){
		// // 	fprintf(stderr,"openAllComms(): inet_pton() failed at line %d\n",currlinePtr->idnum);
		// // 	return 1;
		// // }
		// if(connect(currlinePtr->sock,(struct sockaddr *)&server,sizeof(server))){
		// 	// perror(currlinePtr->ipaddr);
		// 	// fprintf(stderr,"openAllComms(): connect() failed for %s\n",currlinePtr->ipaddr);
		// 	// return 1;
		// }
		// FD_ZERO(&fdset);
	 //    FD_SET(currlinePtr->sock,&fdset);
	 //    if((nfds = select(currlinePtr->sock + 1,NULL,&fdset,NULL,&tv)) == -1){
	 //    	fprintf(stderr,"openAllComms(): select() failed at line %d\n",currlinePtr->idnum);
	 //    	return 1;
	 //    } else if(nfds == 0){
	 //    	fprintf(stderr,"openAllComms(): timeout expired for %s\n",currlinePtr->ipaddr);
	 //    	return 1;
	 //    } else {
	 //        if(getsockopt(currlinePtr->sock,SOL_SOCKET,SO_ERROR,&optval,&optlen) == -1){
	 //        	fprintf(stderr,"openAllComms(): getsockopt() failed at line %d\n",currlinePtr->idnum);
	 //        	return 1;
	 //        }
	 //        if(!optval) {
	 //        	fprintf(stdout,"openAllComms(): %s is open\n",currlinePtr->ipaddr);
	 //        }
	 //    }
		currlinePtr = currlinePtr->next;
	}
	return 0;
}
*/

/*
int closeAllComms(struct Line *topdata){
	struct Line *currlinePtr = topdata;
	while(currlinePtr){
		if(close(currlinePtr->sock)){
			perror(currlinePtr->ipaddr);
			// fprintf(stderr,"closeAllComms(): close() failed at line %d\n",currlinePtr->idnum);
			// return 1;
		} else {
			fprintf(stdout,"closeAllComms(): %s is closed\n",currlinePtr->ipaddr);
		}
		currlinePtr = currlinePtr->next;
	}
	return 0;
}
*/

/* Returns a random hex character */
char randhex(void){
	int num = rand() % 16;
	if(num >=0 && num < 10){
		return '0' + num;
	} else { // 10 <= num <= 15
		return 'a' + num - 10;
	}
}

/* Determines and set command triplets in main data structure to be sent to respective nodes */
int prepCmds(struct Line *datalist, char startdir, struct Path path, char enddir){
	if(path.len == 0){
		fprintf(stderr,"prepCmds(): path must have at least 1 node\n");
		return 1;
	}

	// path.nodes[0]->triplet = malloc(sizeof(*path.nodes[0]->triplet) * 3);
	for(int i = 0; i < path.len - 1; i++){
		isNeighbor(path.nodes[i+1]->idnum,path.nodes[i]->neighbors,&path.nodes[i]->triplet[1]);
		// path.nodes[i]->triplet[2] = '0';
		path.nodes[i]->triplet[2] = randhex();
		// path.nodes[i+1]->triplet = malloc(sizeof(*path.nodes[i+1]->triplet) * 3);
		isNeighbor(path.nodes[i]->idnum,path.nodes[i+1]->neighbors,&path.nodes[i+1]->triplet[0]);
	}
	// path.nodes[path.len - 1]->triplet[2] = '0';
	path.nodes[path.len - 1]->triplet[2] = randhex();

	bool randomize;
	if(startdir != '-'){
		randomize = false;
		if(path.nodes[0]->triplet[1] == startdir){
			fprintf(stderr,"WARNING: startdir needed by path, randomizing over non-path antennas\n");
			randomize = true;
		} else {
			path.nodes[0]->triplet[0] = startdir;
		}
	}
	if(randomize){ // pick random non-path antenna
		int j = 0;
		for(int i = 0; i < path.nodes[0]->ants.num; i++){
			if(path.nodes[0]->ants.array[i] != path.nodes[0]->triplet[1]){
				path.nodes[0]->nonpath.array[j++] = path.nodes[0]->ants.array[i];
			}
		}
		// pick random nonpath.dirs[j]
		path.nodes[0]->triplet[0] = path.nodes[0]->nonpath.array[rand() % path.nodes[0]->nonpath.num];
	}
	
	if(enddir == '-'){ // pick random non-path antenna
		int j = 0;
		for(int i = 0; i < path.nodes[path.len - 1]->ants.num; i++){
			if(path.nodes[path.len - 1]->ants.array[i] != path.nodes[path.len - 1]->triplet[0]){
				path.nodes[path.len - 1]->nonpath.array[j++] = path.nodes[path.len - 1]->ants.array[i];
			}
		}
		// pick random nonpath.dirs[j]
		path.nodes[path.len - 1]->triplet[1] = path.nodes[path.len - 1]->nonpath.array[rand() % path.nodes[path.len - 1]->nonpath.num];
	} else if(enddir == 'o'){ // pick opposite antenna
		bool hasOpp = false;
		char opp = oppDir(path.nodes[path.len - 1]->triplet[0]);
		// printf("lastin is %c, lastout should be opposite: %c\n",path.nodes[path.len - 1]->triplet[0],opp);
		path.nodes[path.len - 1]->triplet[1] = opp;
		for(int i = 0; i < path.nodes[path.len - 1]->ants.num; i++){
			if(path.nodes[path.len - 1]->ants.array[i] == opp){
				hasOpp = true;
				break;
			}
		}
		if(!hasOpp){
			fprintf(stderr,"WARNING: exit antenna does not exist, but set anyway\n");
		}
	} else {
		path.nodes[path.len - 1]->triplet[1] = enddir;
	}
	
	return 0;
}

/* Similar to prepCmds(), but sets all command triplets to "--0" */
int prepOffCmds(struct Line *datalist){
	// char *triplet;
	while(datalist){
		// datalist->triplet = malloc(sizeof(*triplet) * 3);
		datalist->triplet[0] = '-';
		datalist->triplet[1] = '-';
		datalist->triplet[2] = '0';
		datalist = datalist->next;
	}
	return 0;
}

/* Sets quad to {first,second,third,fourth} */
int setQuad(char *quad, char first, char second, char third, char fourth){
	quad[0] = first; quad[1] = second; quad[2] = third; quad[3] = fourth;
	return 0;
}

/* Sets triplet to {first,second,third} */
int setTriplet(char *triplet, char first, char second, char third){
	triplet[0] = first; triplet[1] = second; triplet[2] = third;
	return 0;
}

/* Similar to prepOffCmds, but sets all command triplets to {request[0],request[1],request[2]} */
int prepGivenCmds(struct Line *datalist, char *request){
	while(datalist){
		datalist->triplet[0] = request[0];
		datalist->triplet[1] = request[1];
		datalist->triplet[2] = request[2];
		datalist = datalist->next;
	}
	return 0;
}

/* Sends non-NULL command triplets in main data structure to respective nodes,
   and sets those command triplets to "e-0" */
int sendAllCmds(struct Line *listPtr){
	while(listPtr){
		// if(write(listPtr->host->sock,&listPtr->idnum,1) == -1){
		// 	fprintf(stderr,"sendAllCmds(): write() failed for %s\n",listPtr->host->addr);
		// 	return 1;
		// }
		printf("Sending (%d:%c%c%c) to %s\n",listPtr->idnum,listPtr->triplet[0],listPtr->triplet[1],listPtr->triplet[2],listPtr->host->addr);
		if(listPtr->triplet[0] == listPtr->triplet[1]){
			fprintf(stderr,"sendAllCmds(): cannot set both antenna to %c\n",listPtr->triplet[0]);
			return 1;
		}
		if(sendQuad(listPtr->host->sock,listPtr->idnum,listPtr->triplet)){
			fprintf(stderr,"sendAllCmds(): sendQuad() failed for %s\n",listPtr->host->addr);
			return 1;
		}

		if(listPtr->triplet){
			listPtr->triplet[0] = 'e';
			listPtr->triplet[1] = '-';
			listPtr->triplet[2] = '0';
		}

		listPtr = listPtr->next;
	}
	return 0;
}

/* Listens for and accepts connections on listenSock */
int reopenServ(int listenSock, struct sockaddr_in *server){
	if(listen(listenSock,1)){
		fprintf(stderr,"reopenServ(): listen() failed\n");
		return -1;
	}

	struct sockaddr_in client;
	unsigned int len = sizeof(client), recSock;
	if((recSock = accept(listenSock,(struct sockaddr *)server,&len)) == -1){
		fprintf(stderr,"reopenServ(): accept() failed\n");
		return -1;
	}

	return recSock;
}

/* Generates listenSock, binds it to port, listens on it, and accepts connections
   until all local Arduinos connect (same as openServ3(), but for Arduinos) */
int openServ2(int *listenSock, int port, float timeout, struct sockaddr_in *server, struct Topdata topdata){
	// struct sockaddr_in server;
	if((*listenSock = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("openServ2(){socket()}");
		// perror("SOCKET ERROR: ");
		// fprintf(stderr,"openServ(): socket() failed\n");
		return 1;
	}
	memset(server,0,sizeof(*server));
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = htonl(INADDR_ANY);
	// if(inet_pton(AF_INET,currlinePtr->ipaddr,&server.sin_addr) != 1){
	// 	fprintf(stderr,"openComms(): inet_pton() failed\n");
	// 	return 1;
	// }
	server->sin_port = htons(port);



	// int nfds;
	// fd_set fdset;
	// struct timeval tv;
	// long long usecs = timeout * 1000000;
	// tv.tv_sec = usecs / 1000000;
	// tv.tv_usec = usecs % 1000000;
	// int optval;
	// socklen_t optlen = sizeof(optval);
	// if(fcntl(sock,F_SETFL,O_NONBLOCK) == -1){
	// 	fprintf(stderr,"openComms(): fcntl() failed\n");
	// 	return -1;
	// }

	// printf("About to bind()\n");
	if(bind(*listenSock,(struct sockaddr *)server,sizeof(*server))){
		perror("openServ2(){bind()}");
		// fprintf(stderr,"openServ(): bind() failed\n");
		return 1;
	}

	// printf("About to listen()\n");
	if(listen(*listenSock,topdata.nlocal)){
		perror("openServ2(){listen()}");
		// fprintf(stderr,"openServ(): listen() failed\n");
		return 1;
	}
	// if(connect(sock,(struct sockaddr *)&server,sizeof(server))){
	// 	// fprintf(stderr,"openComms(): connect() failed for %s\n",ipaddr);
	// 	// return 1;
	// }

	struct sockaddr_in client;
	unsigned int len = sizeof(client);
	// int gotSock;
	// int recSock;
	// printf("About to accept()\n");
	// int *socks = malloc(sizeof(*socks) * topdata.nlocal);
	// int sock;
	printf("Accepting %d connections\n",topdata.nlocal);
	for(int i = 0; i < topdata.nlocal; i++){
		printf("Waiting for request...\n");
		if((topdata.localSocks[i] = accept(*listenSock,(struct sockaddr *)server,&len)) == -1){
			perror("openServ2(){accept()}");
			// fprintf(stderr,"openServ(): accept() failed\n");
			return 1;
		}
		struct sockaddr_in *pV4Addr = (struct sockaddr_in *)server;
		struct in_addr ipAddr = pV4Addr->sin_addr;
		char str[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
		printf("Accepted from %s, socket is %d\n",str,topdata.localSocks[i]);
		struct Line *currlinePtr = topdata.list;
		bool assigned = false;
		while(currlinePtr){
			if(!strcmp(currlinePtr->ipaddr,str)){
				currlinePtr->sock = topdata.localSocks[i];
				assigned = true;
				printf("Assigning to #%d\n",currlinePtr->idnum);
				break;
			}
			currlinePtr = currlinePtr->next;
		}
		if(!assigned){
			fprintf(stderr,"openServ2(): could not assign socket\n");
			return 1;
		}
	}

	// FD_ZERO(&fdset);
 //    FD_SET(sock,&fdset);
 //    if((nfds = select(sock + 1,NULL,&fdset,NULL,&tv)) == -1){
 //    	fprintf(stderr,"openComms(): select() failed\n");
 //    	return -1;
 //    } else if(nfds == 0){
 //    	fprintf(stderr,"openComms(): timeout expired\n");
 //    	return -1;
 //    } else {
 //        if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&optval,&optlen) == -1){
 //        	fprintf(stderr,"openComms(): getsockopt() failed\n");
 //        	return -1;
 //        }
 //        if(!optval) {
 //        	fprintf(stdout,"openComms(): successfully opened\n");
 //        }
 //    }

	// close(sock);
	return 0;
}

/* Generates listenSock, binds it to port, listens on it, and accepts connections
   until all local hosts connect (same as openServ2(), but for hosts) */
int openServ3(int *listenSock, int port, float timeout, struct sockaddr_in *server, struct Topdata topdata){
	// struct sockaddr_in server;
	if((*listenSock = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("openServ(){socket()}");
		// perror("SOCKET ERROR: ");
		// fprintf(stderr,"openServ(): socket() failed\n");
		return 1;
	}
	memset(server,0,sizeof(*server));
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = htonl(INADDR_ANY);
	// if(inet_pton(AF_INET,currlinePtr->ipaddr,&server.sin_addr) != 1){
	// 	fprintf(stderr,"openComms(): inet_pton() failed\n");
	// 	return 1;
	// }
	server->sin_port = htons(port);



	// int nfds;
	// fd_set fdset;
	// struct timeval tv;
	// long long usecs = timeout * 1000000;
	// tv.tv_sec = usecs / 1000000;
	// tv.tv_usec = usecs % 1000000;
	// int optval;
	// socklen_t optlen = sizeof(optval);
	// if(fcntl(sock,F_SETFL,O_NONBLOCK) == -1){
	// 	fprintf(stderr,"openComms(): fcntl() failed\n");
	// 	return -1;
	// }

	// printf("About to bind()\n");
	if(bind(*listenSock,(struct sockaddr *)server,sizeof(*server))){
		perror("openServ(){bind()}");
		// fprintf(stderr,"openServ(): bind() failed\n");
		return 1;
	}

	// printf("About to listen()\n");
	if(listen(*listenSock,topdata.nhosts)){
		perror("openServ(){listen()}");
		// fprintf(stderr,"openServ(): listen() failed\n");
		return 1;
	}
	// if(connect(sock,(struct sockaddr *)&server,sizeof(server))){
	// 	// fprintf(stderr,"openComms(): connect() failed for %s\n",ipaddr);
	// 	// return 1;
	// }

	struct sockaddr_in client;
	unsigned int len = sizeof(client);
	// int gotSock;
	// int recSock;
	// printf("About to accept()\n");
	// int *socks = malloc(sizeof(*socks) * topdata.nlocal);
	// int sock;
	printf("Accepting %d connections\n",topdata.nhosts);
	for(int i = 0; i < topdata.nhosts; i++){
		printf("Waiting for request...\n");
		if((topdata.hostSocks[i] = accept(*listenSock,(struct sockaddr *)server,&len)) == -1){
			perror("openServ(){accept()}");
			// fprintf(stderr,"openServ(): accept() failed\n");
			return 1;
		}
		struct sockaddr_in *pV4Addr = (struct sockaddr_in *)server;
		struct in_addr ipAddr = pV4Addr->sin_addr;
		char str[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
		printf("Accepted from %s, socket is %d\n",str,topdata.hostSocks[i]);
		struct Host *currhostPtr = topdata.hosts;
		bool assigned = false;
		while(currhostPtr){
			if(!strcmp(currhostPtr->addr,str)){
				currhostPtr->sock = topdata.hostSocks[i];
				assigned = true;
				printf("Assigning to %s\n",currhostPtr->addr);
				break;
			}
			currhostPtr = currhostPtr->next;
		}
		if(!assigned){
			fprintf(stderr,"openServ(): could not assign socket\n");
			return 1;
		}
	}

	// FD_ZERO(&fdset);
 //    FD_SET(sock,&fdset);
 //    if((nfds = select(sock + 1,NULL,&fdset,NULL,&tv)) == -1){
 //    	fprintf(stderr,"openComms(): select() failed\n");
 //    	return -1;
 //    } else if(nfds == 0){
 //    	fprintf(stderr,"openComms(): timeout expired\n");
 //    	return -1;
 //    } else {
 //        if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&optval,&optlen) == -1){
 //        	fprintf(stderr,"openComms(): getsockopt() failed\n");
 //        	return -1;
 //        }
 //        if(!optval) {
 //        	fprintf(stdout,"openComms(): successfully opened\n");
 //        }
 //    }

	// close(sock);
	return 0;
}

/* Returns socket from single acceptance */
int openServ(int *listenSock, int port, float timeout, struct sockaddr_in *server){
	// struct sockaddr_in server;
	if((*listenSock = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("openServ(){socket()}");
		// perror("SOCKET ERROR: ");
		// fprintf(stderr,"openServ(): socket() failed\n");
		return -1;
	}
	memset(server,0,sizeof(*server));
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = htonl(INADDR_ANY);
	// if(inet_pton(AF_INET,currlinePtr->ipaddr,&server.sin_addr) != 1){
	// 	fprintf(stderr,"openComms(): inet_pton() failed\n");
	// 	return 1;
	// }
	server->sin_port = htons(port);



	// int nfds;
	// fd_set fdset;
	// struct timeval tv;
	// long long usecs = timeout * 1000000;
	// tv.tv_sec = usecs / 1000000;
	// tv.tv_usec = usecs % 1000000;
	// int optval;
	// socklen_t optlen = sizeof(optval);
	// if(fcntl(sock,F_SETFL,O_NONBLOCK) == -1){
	// 	fprintf(stderr,"openComms(): fcntl() failed\n");
	// 	return -1;
	// }

	// printf("About to bind()\n");
	if(bind(*listenSock,(struct sockaddr *)server,sizeof(*server))){
		perror("openServ(){bind()}");
		// fprintf(stderr,"openServ(): bind() failed\n");
		return -1;
	}

	// printf("About to listen()\n");
	if(listen(*listenSock,1)){
		perror("openServ(){listen()}");
		// fprintf(stderr,"openServ(): listen() failed\n");
		return -1;
	}
	// if(connect(sock,(struct sockaddr *)&server,sizeof(server))){
	// 	// fprintf(stderr,"openComms(): connect() failed for %s\n",ipaddr);
	// 	// return 1;
	// }

	struct sockaddr_in client;
	unsigned int len = sizeof(client), recSock;
	// printf("About to accept()\n");
	if((recSock = accept(*listenSock,(struct sockaddr *)server,&len)) == -1){
		perror("openServ(){accept()}");
		// fprintf(stderr,"openServ(): accept() failed\n");
		return -1;
	}

	// FD_ZERO(&fdset);
 //    FD_SET(sock,&fdset);
 //    if((nfds = select(sock + 1,NULL,&fdset,NULL,&tv)) == -1){
 //    	fprintf(stderr,"openComms(): select() failed\n");
 //    	return -1;
 //    } else if(nfds == 0){
 //    	fprintf(stderr,"openComms(): timeout expired\n");
 //    	return -1;
 //    } else {
 //        if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&optval,&optlen) == -1){
 //        	fprintf(stderr,"openComms(): getsockopt() failed\n");
 //        	return -1;
 //        }
 //        if(!optval) {
 //        	fprintf(stdout,"openComms(): successfully opened\n");
 //        }
 //    }

	// close(sock);
	return recSock;
}

/* Connects to the given ipaddr on the given port */
int openClient(char *ipaddr, int port, float timeout){
	int sock;
	struct sockaddr_in server;
	if((sock = socket(AF_INET,SOCK_STREAM,0)) == -1){
		// fprintf(stderr,"openClient(): socket() failed\n");
		perror("openClient(){socket()}");
		return -1;
	}
	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	// server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(inet_pton(AF_INET,ipaddr,&server.sin_addr) != 1){
		// fprintf(stderr,"openClient(): inet_pton() failed\n");
		perror("openClient{inet_pton()}");
		return -1;
	}
	server.sin_port = htons(port);



	// int nfds;
	// fd_set fdset;
	// struct timeval tv;
	// long long usecs = timeout * 1000000;
	// tv.tv_sec = usecs / 1000000;
	// tv.tv_usec = usecs % 1000000;
	// int optval;
	// socklen_t optlen = sizeof(optval);
	// if(fcntl(sock,F_SETFL,O_NONBLOCK) == -1){
	// 	fprintf(stderr,"openComms(): fcntl() failed\n");
	// 	return -1;
	// }

	// if(bind(sock,(struct sockaddr *)&server,sizeof(server))){
	// 	fprintf(stderr,"openServ(): bind() failed\n");
	// 	return -1;
	// }

	// if(listen(sock,1)){
	// 	fprintf(stderr,"openServ(): listen() failed\n");
	// 	return -1;
	// }

	if(connect(sock,(struct sockaddr *)&server,sizeof(server))){
		perror(ipaddr);
		// fprintf(stderr,"openClient(): connect() failed for %s\n",ipaddr);
		return -1;
	}

	// struct sockaddr_in client;
	// int len = sizeof(client), sockfd;
	// if((sockfd = accept(sock,(struct sockaddr *)&server,&len)) == -1){
	// 	fprintf(stderr,"openServ(): accept() failed\n");
	// 	return -1;
	// }

	// FD_ZERO(&fdset);
 //    FD_SET(sock,&fdset);
 //    if((nfds = select(sock + 1,NULL,&fdset,NULL,&tv)) == -1){
 //    	fprintf(stderr,"openComms(): select() failed\n");
 //    	return -1;
 //    } else if(nfds == 0){
 //    	fprintf(stderr,"openComms(): timeout expired\n");
 //    	return -1;
 //    } else {
 //        if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&optval,&optlen) == -1){
 //        	fprintf(stderr,"openComms(): getsockopt() failed\n");
 //        	return -1;
 //        }
 //        if(!optval) {
 //        	fprintf(stdout,"openComms(): successfully opened\n");
 //        }
 //    }

	return sock; //sockfd?
}

/* Reads an incoming command quad from the given socket */
int recCmd(int recSock, struct Cmd *cmd){
	char quad[4];
	int nrec = read(recSock,quad,4);
	if(!nrec){
		return 0;
	} else if(nrec == -1){
		fprintf(stderr,"recCmd(): read() failed\n");
		return -1;
	} else if(nrec != 4){
		fprintf(stderr,"recCmd(): read() failed to fill quad (only got %d/4)\n",nrec);
		return -1;
	} else {
		// printf("About to set cmd->idnum to %d\n",(int)quad[0]);
		// printf("About to set cmd->triplet[0] to %c",quad[1]);
		// printf("About to set cmd->triplet[1] to %c",quad[2]);
		// printf("About to set cmd->triplet[2] to %c",quad[3]);
		cmd->idnum = (int)quad[0];
		cmd->triplet[0] = quad[1];
		cmd->triplet[1] = quad[2];
		cmd->triplet[2] = quad[3];
		return 1;
	}
}

/* Returns the average of array vVals */
float average(float *vVals, int nTrials){
	float sum = 0;
	for(int i = 0; i < nTrials; i++){
		sum += vVals[i];
	}
	return sum /= nTrials;
}

/* Reads incoming data from the given Arduino socket */
int queryRecv(int sock, struct Data *data){
	// char request[3] = {dir,'-','v'};
	// if(sendTriplet(sock,request)){
	// 	fprintf(stderr,"queryUnit(): sendTriplet() failed\n");
	// 	return -1;
	// }

	// fprintf(stderr,"About to read\n");
	// printf("sizeof(struct Data) is %ld\n",sizeof(struct Data));
	int nrec = read(sock,data,ARDUINO_SIZEOF_STRUCT_DATA);
	if(!nrec){
		return 0;
	} else if(nrec == -1){
		fprintf(stderr,"queryRecv(): read() failed\n");
		return -1;
	} else if(nrec != ARDUINO_SIZEOF_STRUCT_DATA){
		fprintf(stderr,"queryRecv(): read() failed to fill data (only got %d/%d)\n",nrec,ARDUINO_SIZEOF_STRUCT_DATA);
		return -1;
	}

	return 1;
}

/* Sends a request to the given Arduino socket and reads the reply */
int queryUnit(int sock, char dir, struct Data *data){
	char request[3] = {dir,'-','v'};
	if(sendTriplet(sock,request)){
		fprintf(stderr,"queryUnit(): sendTriplet() failed\n");
		return -1;
	}

	// fprintf(stderr,"About to read\n");
	// printf("sizeof(struct Data) is %ld\n",sizeof(struct Data));
	int nrec = read(sock,data,ARDUINO_SIZEOF_STRUCT_DATA);
	if(!nrec){
		return 0;
	} else if(nrec == -1){
		fprintf(stderr,"queryRecv(): read() failed\n");
		return -1;
	} else if(nrec != ARDUINO_SIZEOF_STRUCT_DATA){
		fprintf(stderr,"queryRecv(): read() failed to fill data (only got %d/%d)\n",nrec,ARDUINO_SIZEOF_STRUCT_DATA);
		return -1;
	}

	return 1;
}

/* Returns the median of the array dataBlocks associated with the given idnum */
struct Report getMedian(struct Data *dataBlocks, int idnum, int ntrials){
	struct Report median;
	median.idnum = idnum;
	selectionSort(dataBlocks,ntrials);
	median.data = dataBlocks[((ntrials - 1) / 2)];
	return median;
}

/* Returns the median of the array trialsData */
float getMedian2(float *trialsData, int ntrials){
	qsort(trialsData,ntrials,sizeof(*trialsData),compare);
	return trialsData[(ntrials - 1) / 2];
}

/* Generic comparator */
int compare(const void *arg1, const void *arg2){
	float lhs = *((float *)arg1);
	float rhs = *((float *)arg2);
    if(lhs < rhs){
    	return -1;
    } else if(lhs > rhs){
    	return 1;
    } else {
    	return 0;
    }
}

/* Implementation of selection sort for median calculation */
void selectionSort(struct Data *dataBlocks, int ntrials){
	struct Data min;
	int iMin;
	// int i;

	// printf("Original:");
	// for(int i = 0; i < ntrials; i++){
	// 	printf(" %f ",dataBlocks[i].val);
	// }
	// printf("\n");

	for(int dest = 0; dest < ntrials - 1; dest++){
		min = dataBlocks[dest];
		iMin = dest;
		for(int i = dest + 1; i < ntrials; i++){
			if(dataBlocks[i].val < min.val){
				min = dataBlocks[i];
				iMin = i;
			}
		}

		dataBlocks[iMin] = dataBlocks[dest];
		dataBlocks[dest] = min;

		// printf("Moving %d to %d, and putting temp in %d\n",iMin,dest,iMin);
		// temp = dataBlocks[dest];
		// dataBlocks[dest] = dataBlocks[iMin];
		// dataBlocks[iMin] = temp;
		// printf("Setting %d to %d, and %d to min\n",iMin,dest,dest);
		// dataBlocks[iMin] = dataBlocks[dest];
		// dataBlocks[dest] = min;

		// printf("List now:");
		// for(int i = 0; i < ntrials; i++){
		// 	printf(" %f ",dataBlocks[i].val);
		// }
		// printf("\n");
	}

	// int dest = 0;
	// struct Data temp;
	// for(int i = 1; i < ntrials; i++){
	// 	if(dataBlocks[i].val < dataBlocks[dest].val){
	// 		printf("Swapping %d and %d\n",i,dest);
	// 		temp = dataBlocks[dest];
	// 		dataBlocks[dest] = dataBlocks[i];
	// 		dataBlocks[i] = temp;
	// 		i = ++dest; // will be incremented by for loop
	// 	}
	// }
	printf("Sorted list:");
	for(int i = 0; i < ntrials; i++){
		printf(" %f ",dataBlocks[i].val);
	}
	printf("\n");
	return;
}

// int queryBestVoltage(int sock, struct Max *vBlock, struct Max *pBlock){
// 	/* Simulation */
// 	vBlock->val = 10;
// 	vBlock->dir = 'n';
// 	pBlock->val = 20;
// 	pBlock->dir = 's';
// 	return 1;

// 	char request[3] = {'-','-','v'};
// 	if(sendTriplet(sock,request)){
// 		fprintf(stderr,"queryVoltage(): sendTriplet() failed\n");
// 		return -1;
// 	}

// 	// fprintf(stderr,"About to read\n");
// 	int nrec = read(sock,vBlock,sizeof(*vBlock));
// 	if(!nrec){
// 		return 0;
// 	} else if(nrec == -1){
// 		fprintf(stderr,"queryVoltage(): read() failed\n");
// 		return -1;
// 	} else if(nrec != sizeof(*vBlock)){
// 		fprintf(stderr,"queryVoltage(): read() failed to fill voltage (only got %d/%ld)\n",nrec,sizeof(*vBlock));
// 		return -1;
// 	}

// 	nrec = read(sock,pBlock,sizeof(*pBlock));
// 	if(!nrec){
// 		return 0;
// 	} else if(nrec == -1){
// 		fprintf(stderr,"queryVoltage(): read() failed\n");
// 		return -1;
// 	} else if(nrec != sizeof(*pBlock)){
// 		fprintf(stderr,"queryVoltage(): read() failed to fill powerIndex (only got %d/%ld)\n",nrec,sizeof(*pBlock));
// 		return -1;
// 	}

// 	return 1;
// }

/* Reads data from sock into report */
int getReport(int sock, struct Report *report){
	// char request[3] = {dir,mid,type};
	// if(sendQuad(sock,idnum,request)){
	// 	fprintf(stderr,"getReport(): sendQuad() failed\n");
	// 	return 1;
	// }

	int nrec = read(sock,report,sizeof(*report));
	if(!nrec){
		return 0;
	} else if(nrec == -1){
		fprintf(stderr,"getReport(): read() failed\n");
		return -1;
	} else if(nrec != sizeof(*report)){
		fprintf(stderr,"getReport(): read() failed to fill report (only got %d/%ld)\n",nrec,sizeof(*report));
		return -1;
	}

	return 1;
}

/* Writes data from report to sock */
int report(int sock, struct Report report){
	// fprintf(stderr,"report.idnumV is %d\n",report.idnumV);
	// fprintf(stderr,"report.idnumP is %d\n",report.idnumP);
	// fprintf(stderr,"report.vMax.dir is %c\n",report.vMax.dir);
	// fprintf(stderr,"report.vMax.val is %f\n",report.vMax.val);
	// fprintf(stderr,"report.pMax.dir is %c\n",report.pMax.dir);
	// fprintf(stderr,"report.pMax.val is %f\n",report.pMax.val);
	int nsent = write(sock,&report,sizeof(report));
	if(nsent == -1){
		fprintf(stderr,"report(): write() failed\n");
		return 1;
	} else if(nsent != sizeof(report)){
		fprintf(stderr,"report(): write() failed to send report\n");
		return 1;
	} else {
		return 0;
	}
}

/* Sends <idnum><triplet> to sock */
int sendQuad(int sock, int idnum, char *triplet){
	printf("Sending %c%c%c\n",triplet[0],triplet[1],triplet[2]);
	char quad[4] = {(char)idnum,triplet[0],triplet[1],triplet[2]};
	int nsent = write(sock,quad,4);
	if(nsent == -1){
		fprintf(stderr,"sendQuad(): write() failed\n");
		return 1;
	} else if(nsent != 4){
		fprintf(stderr,"sendQuad(): write() failed to send full quad\n");
		return 1;
	} else {
		return 0;
	}
	// fprintf(stderr,"Just wrote: ");
	// if(write(STDERR_FILENO,triplet,3) == -1){
	// 	fprintf(stderr,"sendTriplet(): write() failed\n");
	// 	return 1;
	// }
	// fprintf(stderr," to socket %d\n",sock);
	// return 0;
}

/* Sends quad to sock */
int sendQuad2(int sock, char *quad){
	// printf("sendCmd(): sending %c%c%c\n",triplet[0],triplet[1],triplet[2]);
	// char quad[4] = {(char)idnum,triplet[0],triplet[1],triplet[2]};
	int nsent = write(sock,quad,4);
	if(nsent == -1){
		fprintf(stderr,"sendQuad(): write() failed\n");
		return 1;
	} else if(nsent != 4){
		fprintf(stderr,"sendQuad(): write() failed to send full quad\n");
		return 1;
	} else {
		return 0;
	}
	// fprintf(stderr,"Just wrote: ");
	// if(write(STDERR_FILENO,triplet,3) == -1){
	// 	fprintf(stderr,"sendTriplet(): write() failed\n");
	// 	return 1;
	// }
	// fprintf(stderr," to socket %d\n",sock);
	// return 0;
}

/* Writes data at ptr to sock */
int genericSend(int sock, void *ptr, size_t count){
	int nsent = write(sock,ptr,count);
	if(nsent == -1){
		fprintf(stderr,"genericSend(): write() failed\n");
		return 1;
	} else if(nsent != count){
		fprintf(stderr,"sendQuad(): only wrote %d/%ld\n",nsent,count);
		return 1;
	} else {
		return 0;
	}
}

/* Sends triplet to sock */
int sendTriplet(int sock, char *triplet){
	// printf("sendCmd(): sending %c%c%c\n",triplet[0],triplet[1],triplet[2]);
	int nsent = write(sock,triplet,3);
	if(nsent == -1){
		fprintf(stderr,"sendTriplet(): write() failed\n");
		return 1;
	} else if(nsent != 3){
		fprintf(stderr,"sendTriplet(): write() failed to send full triplet\n");
		return 1;
	} else {
		return 0;
	}
	// fprintf(stderr,"Just wrote: ");
	// if(write(STDERR_FILENO,triplet,3) == -1){
	// 	fprintf(stderr,"sendTriplet(): write() failed\n");
	// 	return 1;
	// }
	// fprintf(stderr," to socket %d\n",sock);
	// return 0;
}

// int closeComms(int sock){
// 	if(close(sock)){
// 		fprintf(stderr,"closeComms(): close() failed\n");
// 		// return 1;
// 	}
// 	// } else {
// 	// 	fprintf(stdout,"closeComms(): successfully closed\n");
// 	// }
// 	return 0;
// }

/* Adds abstract line to end of path */
int pathPush(struct Path *path, struct Line *linePtr){
	path->nodes = realloc(path->nodes,sizeof(*path->nodes) * ++path->len);
	path->nodes[path->len - 1] = linePtr;
	return 0;
}

/* Adds path to pathset */
int savePath(struct Path *path, struct Paths *pathset){
	pathset->paths = realloc(pathset->paths,sizeof(*pathset->paths) * ++pathset->count);
	pathcpy(&pathset->paths[pathset->count - 1],path);
	return 0;
}

/* Removes abstract line from end of path */
int pathPop(struct Path *path){
	path->nodes = realloc(path->nodes,sizeof(*path->nodes) * --path->len);
	return 0;
}

/* Dumps the path to the stream (e.g., stdout) */
int printPath(FILE *stream, struct Path path){
	int i;
	for(i = 0; i < path.len - 1; i++){
		fprintf(stream,"%d,",path.nodes[i]->idnum);
	}
	fprintf(stream,"%d\n",path.nodes[i]->idnum);
	return 0;
}

/* Frees the set of paths */
int freePaths(struct Paths pathset){
	for(int i = 0; i < pathset.count; i++){
		free(pathset.paths[i].nodes);
	}
	free(pathset.paths);
	return 0;
}

/* Dumps the set of paths to the stream (e.g., stdout) */
int printPaths(FILE *stream, struct Paths allpaths){
	for(int i = 0; i < allpaths.count; i++){
		printPath(stream, allpaths.paths[i]);
	}
	return 0;
}

/* Allocates memory and fills it with copy of source path */
int pathcpy(struct Path *dest, struct Path *src){
	dest->len = src->len;
	dest->nodes = malloc(sizeof(*dest->nodes) * src->len);
	for(int i = 0; i < src->len; i++){
		dest->nodes[i] = src->nodes[i];
	}
	return 0;
}

/* For program that takes in manual links, converts parameters to abstract form */
int parseLinks(struct Links *links, char *line){
	// printf("Parsing: %s\n",line);
	if(line == NULL){
		fprintf(stderr,"parseLinks(): line is NULL\n");
		return 1;
	}
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
	links->nlinks = 0;
	links->linkset = NULL;
	char *ptr = line;
	while(ptr[0] != '\0'){
		links->linkset = realloc(links->linkset,sizeof(*links->linkset) * (links->nlinks + 1));
		links->linkset[links->nlinks].startid = (int)strtol(ptr,&ptr,10);
		// printf("setting startid[%d] to %d\n",links->nlinks,links->linkset[links->nlinks].startid);
		links->linkset[links->nlinks].startdir = ptr[1];
		// printf("setting startdir[%d] to %c\n",links->nlinks,links->linkset[links->nlinks].startdir);
		links->linkset[links->nlinks].endid = (int)strtol(ptr+3,&ptr,10);
		// printf("setting endid[%d] to %d\n",links->nlinks,links->linkset[links->nlinks].endid);
		links->linkset[links->nlinks].enddir = ptr[1];
		// printf("setting enddir[%d] to %c\n",links->nlinks,links->linkset[links->nlinks].enddir);
		links->nlinks++;
		ptr += 3;
	}
	return 0;
}

/* Loops over hosts and sends quad to each */
int broadcast(struct Host *hosts, char *quad){
	struct Host *currhostPtr = hosts;
	while(currhostPtr){
		if(sendQuad2(currhostPtr->sock,quad)){
			fprintf(stderr,"broadcast(): sendQuad2() failed for %s\n",currhostPtr->addr);
			return 1;
		}
		currhostPtr = currhostPtr->next;
	}
	return 0;
}

/* Loops over hosts and sends data at ptr to each */
int genericBroadcast(struct Host *hosts, void *ptr, size_t count){
	struct Host *currhostPtr = hosts;
	while(currhostPtr){
		if(genericSend(currhostPtr->sock,ptr,count)){
			fprintf(stderr,"genericBroadcast(): genericSend() failed for %s\n",currhostPtr->addr);
			return 1;
		}
		currhostPtr = currhostPtr->next;
	}
	return 0;
}

/* Sets endptPtr to the highest-scoring neighbor of spikePtr within 2 hops */
int bestNearby(struct Line *spikePtr, int ntrials, struct Endpoint *endptPtr){
	/* get score of spikePtr and initialize *endptPtr */
	struct Data data;
	if(getUnitData(spikePtr,'-',ntrials,&data)){
		fprintf(stderr,"bestNearby(): getData() failed\n");
		return 1;
	}
	endptPtr->line = spikePtr;
	endptPtr->data = data;
	struct Line *nearNeighbor, *farNeighbor;
	for(int i = 0; i < spikePtr->neighbors.count; i++){
		nearNeighbor = spikePtr->neighbors.lines[i];
		/* if not already queried, get score of nearNeighbor; if best so far, retain */
		if(!nearNeighbor->queried){
			nearNeighbor->queried = true;
			if(getUnitData(nearNeighbor,'-',ntrials,&data)){
				fprintf(stderr,"bestNearby(): getData() failed\n");
				return 1;
			}
			if(data.val > endptPtr->data.val){
				endptPtr->line = nearNeighbor;
				endptPtr->data = data;
			}
		}
		for(int j = 0; j < nearNeighbor->neighbors.count; j++){
			farNeighbor = nearNeighbor->neighbors.lines[i];
			/* if not already queried, get score of farNeighbor; if best so far, retain */
			if(!farNeighbor->queried){
				farNeighbor->queried = true;
				if(getUnitData(farNeighbor,'-',ntrials,&data)){
					fprintf(stderr,"bestNearby(): getData() failed\n");
					return 1;
				}
				if(data.val > endptPtr->data.val){
					endptPtr->line = farNeighbor;
					endptPtr->data = data;
				}
			}
		}
	}

	/* Reset queried indicators */
	for(int i = 0; i < spikePtr->neighbors.count; i++){
		nearNeighbor = spikePtr->neighbors.lines[i];
		nearNeighbor->queried = false;
		for(int j = 0; j < nearNeighbor->neighbors.count; j++){
			farNeighbor = nearNeighbor->neighbors.lines[i];
			farNeighbor->queried = false;
		}
	}

	return 0;
}

/* Sends <idnum><dir>g to relevant sock and reads answer into dataPtr */
int getUnitData(struct Line *linePtr, char dir, int ntrials, struct Data *dataPtr){
	char quad[4];
	setQuad(quad,(char)linePtr->idnum,dir,(char)ntrials,'g');
	if(sendQuad2(linePtr->host->sock,quad)){ // host ???
		fprintf(stderr,"broadcast(): sendQuad2() failed for %s\n",linePtr->host->addr);
		return 1;
	}
	int stat = readAnswer(linePtr->host->sock,dataPtr,sizeof(*dataPtr));
	if(!stat){
		fprintf(stderr,"getUnitData(): readAnswer() failed to read from %s\n",linePtr->host->addr);
		return 1;
	} else if(stat == -1){
		fprintf(stderr,"getUnitData(): readAnswer() failed for %s\n",linePtr->host->addr);
		return 1;
	}
	return 0;
}

/* Reads count bytes from sock into ptr */
int readAnswer(int sock, void *ptr, size_t count){
	int nrec = read(sock,ptr,count);
	if(!nrec){
		return 0;
	} else if(nrec == -1){
		fprintf(stderr,"readAnswer(): read() failed\n");
		return -1;
	} else if(nrec != count){
		fprintf(stderr,"readAnswer(): read() failed to fill ... (only got %d/%ld)\n",nrec,count);
		return -1;
	}

	return 1;
}

/* Waits for message from any sock in hostSocks, with timeout */
int waitAlert(struct Topdata topdata, int *spikeIDptr, float alertDuration){
	struct timeval *alertTimeout = NULL;
	struct timeval tv;
	if(alertDuration >= 0){
		// alertTimeout = malloc(sizeof(*alertTimeout));
		tv.tv_sec = (int)alertDuration;
		alertDuration = (alertDuration - tv.tv_sec) * 1000000;
		tv.tv_usec = (int)alertDuration;
		alertTimeout = &tv;
	}

	fd_set sockSet;
	FD_ZERO(&sockSet);
	// int maxHost;
	for(int i = 0; i < topdata.nhosts; i++){
		// if(!i){
		// 	maxHost = topdata.hostSocks[i];
		// } else if(topdata.hostSocks[i] > maxHost){
		// 	maxHost = topdata.hostSocks[i];
		// }
		FD_SET(topdata.hostSocks[i],&sockSet);
	}
	int stat = select(topdata.maxHost + 1,&sockSet,NULL,NULL,alertTimeout);
	if(!stat){
		fprintf(stderr,"waitAlert(): alertTimeout expired\n");
		return -1;
	} else if(stat == -1){
		perror("waitAlert(){select()}");
		return -1;
	}

	/* Stop listening */
	// char signal = 's';
	// if(genericBroadcast(topdata.hosts,&signal,1)){
	// 	fprintf(stderr,"waitAlert(): genericBroadcast() failed\n");
	// 	exit(1);
	// }

	// int sock;
	// for(int i = 0; i < topdata.nhosts; i++){
	// 	if(FD_ISSET(topdata.hostSocks[i],&sockSet)){
	// 		sock = topdata.hostSocks[i];
	// 		// FD_ZERO(&sockSet);
	// 		break;
	// 	}
	// }

	// stat = readAnswer(sock,spikeIDptr,sizeof(*spikeIDptr));
	// if(!stat){
	// 	fprintf(stderr,"getUpdates(): readAnswer() failed to read\n");
	// 	return 1;
	// } else if(stat == -1){
	// 	fprintf(stderr,"getUpdates(): readAnswer() failed\n");
	// 	return 1;
	// }

	return 1;
}

/* Waits for message from any sock in sockSet, with timeout */
int waitAlert2(struct Topdata topdata, int *spikeIDptr, float alertDuration){
	struct timeval *alertTimeout = NULL;
	struct timeval tv;
	if(alertDuration >= 0){
		// alertTimeout = malloc(sizeof(*alertTimeout));
		tv.tv_sec = (int)alertDuration;
		alertDuration = (alertDuration - tv.tv_sec) * 1000000;
		tv.tv_usec = (int)alertDuration;
		alertTimeout = &tv;
	}
	
	fd_set sockSet;
	FD_ZERO(&sockSet);
	struct Line *currlinePtr = topdata.list;
	int maxSock;
	bool gotFirst = false;
	while(currlinePtr){
		if(!strcmp(currlinePtr->host->addr,topdata.inet_addr)){
			if(!gotFirst){
				gotFirst = true;
				maxSock = currlinePtr->sock;
			} else if(currlinePtr->sock > maxSock){
				maxSock = currlinePtr->sock;
			}
			FD_SET(currlinePtr->sock,&sockSet);
		}
		currlinePtr = currlinePtr->next;
	}
	int stat = select(maxSock + 1,&sockSet,NULL,NULL,alertTimeout);
	if(!stat){
		fprintf(stderr,"waitAlert2(): alertTimeout expired\n");
		return -1;
	} else if(stat == -1){
		perror("waitAlert2(){select()}");
		return -1;
	}

	/* Stop listening */
	char signal = 's';
	if(genericBroadcast(topdata.hosts,&signal,1)){
		fprintf(stderr,"waitAlert(): genericBroadcast() failed\n");
		exit(1);
	}

	int sock;
	for(int i = 0; i < topdata.nhosts; i++){
		if(FD_ISSET(topdata.hostSocks[i],&sockSet)){
			sock = topdata.hostSocks[i];
			// FD_ZERO(&sockSet);
			break;
		}
	}

	stat = readAnswer(sock,spikeIDptr,sizeof(*spikeIDptr));
	if(!stat){
		fprintf(stderr,"getUpdates(): readAnswer() failed to read\n");
		return 1;
	} else if(stat == -1){
		fprintf(stderr,"getUpdates(): readAnswer() failed\n");
		return 1;
	}

	return 1;
}

/* Sends <idnum><dir><pathAnt><u> to Arduino and reads data on loop until
   reported score is too low */
int getUpdates(struct Endpoint endpt, float frac, char pathAnt, int redundancy){
	/* Request continuous updates */
	char quad[4];
	setQuad(quad,(char)endpt.line->idnum,endpt.data.dir,pathAnt,'u');
	if(sendQuad2(endpt.line->host->sock,quad)){
		fprintf(stderr,"getUpdates(): sendQuad2() failed for %s\n",endpt.line->host->addr);
		return 1;
	}

	int stat, nlow = 0;
	float score;
	char signal = 'h';
	while(1){
		stat = readAnswer(endpt.line->host->sock,&score,sizeof(score));
		if(!stat){
			fprintf(stderr,"getUpdates(): readAnswer() failed to read from %s\n",endpt.line->host->addr);
			return 1;
		} else if(stat == -1){
			fprintf(stderr,"getUpdates(): readAnswer() failed for %s\n",endpt.line->host->addr);
			return 1;
		}

		if(score < endpt.data.val){
			if(++nlow == redundancy){
				/* Halt updates */
				if(genericSend(endpt.line->host->sock,&signal,1)){
					fprintf(stderr,"getUpdates(): genericSend() failed for #%d\n",endpt.line->idnum);
					exit(1);
				}
				break;
			}
		} else {
			nlow = 0;
		}
	}
	
	return 0;
}

/* Returns -1 on error or timeout, 1 if received, 0 otherwise */
int gotSignal(int sock){
	fd_set sockSet;
	FD_ZERO(&sockSet);
	FD_SET(sock,&sockSet);
	struct timeval tv;
	tv.tv_sec = tv.tv_usec = 0;
	int stat = select(sock + 1,&sockSet,NULL,NULL,&tv);
	if(!stat){
		fprintf(stderr,"gotSignal(): timeout expired\n");
		return -1;
	} else if(stat == -1){
		perror("gotSignal(){select()}");
		return -1;
	}

	if(FD_ISSET(sock,&sockSet)){
		// FD_ZERO(&sockSet);
		return 1;
	} else {
		// FD_ZERO(&sockSet);
		return 0;
	}
}

/* Runs in the background of the subcontrollers. Reads data as it arrives from set of sockets.
   As messages arrive, they are parsed and forwarded to the master. */
void *updateTable(void *topdataPtrVoid){
	struct Topdata *topdataPtr = (struct Topdata *)topdataPtrVoid;
	int nready, stat;
	fd_set sockSet;
	struct timeval *alertTimeout, tv;
	float alertDuration;
	// struct Line *alertPtr;
	struct Message2 message;
	memset(&message,0,sizeof(message));
	message.score = -1;
	message.idnum = -1;
	message.antenna = -1;
	// bool gotFirst = false;
	// int maxSock;

	alertTimeout = NULL;
	if(topdataPtr->alertTimeout >= 0){
		tv.tv_sec = (int)topdataPtr->alertTimeout;
		alertDuration = (topdataPtr->alertTimeout - tv.tv_sec) * 1000000;
		tv.tv_usec = (int)alertDuration;
		alertTimeout = &tv;
		// printf("%ld seconds, %ld microseconds\n",tv.tv_sec,tv.tv_usec);
	}
	// FD_ZERO(&sockSet);
	// for(int i = 0; i < topdataPtr->nhosts; i++){
	// 	FD_SET(topdataPtr->hostSocks[i],&sockSet);
	// }
	// struct Line *currlinePtr = topdataPtr->list;
	// while(currlinePtr){
	// 	if(!strcmp(currlinePtr->host->addr,topdataPtr->inet_addr)){
	// 		if(!gotFirst){
	// 			gotFirst = true;
	// 			maxSock = currlinePtr->sock;
	// 		} else {
	// 			if(currlinePtr->sock > maxSock){
	// 				maxSock = currlinePtr->sock;
	// 			}
	// 		}
	// 		// FD_SET(currlinePtr->sock,&sockSet);
	// 	}
	// 	currlinePtr = currlinePtr->next;
	// }

	// if(!gotFirst){
	// 	fprintf(stderr,"updateTable(): nothing to listen to\n");
	// 	return NULL; //***
	// }

	struct Line *currlinePtr;
	// bool gotBoth = false;
	// bool needEnd1 = true;
	// bool needEnd2 = false;
	// int end1, end2;

	while(1){
		// printf("\t\t\tBackground\n");
		/* Detect incoming message(s) */
		FD_ZERO(&sockSet);
		currlinePtr = topdataPtr->list;
		while(currlinePtr){
			if(!strcmp(currlinePtr->host->addr,topdataPtr->inet_addr)){
				// printf("Currently #%d\n",currlinePtr->idnum);
				// printf("Its sock is: %d\n",currlinePtr->sock);
				FD_SET(currlinePtr->sock,&sockSet);
			}
			currlinePtr = currlinePtr->next;
		}

		// printf("Waiting for message...\n");
		nready = select(topdataPtr->maxLocal + 1,&sockSet,NULL,NULL,alertTimeout);
		if(!nready){
			fprintf(stderr,"updateTable(): alertTimeout expired\n");
			exit(1);
		} else if(nready == -1){
			perror("updateTable(){select()}");
			exit(1);
		}

		// printf("\t\t\tBackground: nready is %d\n",nready);

		struct Message report;

		/* Process incoming messages */
		// for(int i = 0; i < nready; i++){
		for(int i = 0; i < topdataPtr->nlocal; i++){
			if(FD_ISSET(topdataPtr->localSocks[i],&sockSet)){
				stat = readAnswer(topdataPtr->localSocks[i],(uint8_t *)&report,sizeof(report));
				if(!stat){
					fprintf(stderr,"updateTable(): readAnswer() failed to read\n");
					exit(1);
				} else if(stat == -1){
					fprintf(stderr,"updateTable(): readAnswer() failed\n");
					exit(1);
				}

				if(gettimeofday(&message.time,NULL)){
					perror("updateTable(){gettimeofday()}");
					exit(1);
				}
				// printf("currtime: %f\n",floatTV(message.time));

				message.idnum = report.idnum;
				message.score = report.score;
				message.antenna = report.antenna;

				
				// sleep(1);
				// message.idnum = 1;
				// message.score = 400;

				if(topdataPtr->gotCommand){

 /* Added by Ivan */

 struct timeval tv;
 struct tm* ptm;
 char time_string[40];
 long microseconds;

 /* Obtain the time of day, and convert it to a tm struct. */
 gettimeofday (&tv, NULL);
 ptm = localtime (&tv.tv_sec);
 /* Format the date and time, down to a single second. */
 strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
 /* Compute milliseconds from microseconds. */
 microseconds = tv.tv_usec;
 /* Print the formatted time, in seconds, followed by a decimal point
   and the milliseconds. */


					printf("Received (%d,%c,%d,%s.%04ld)\n",message.idnum,(char)message.antenna,message.score,time_string, microseconds);

					/* Forward to master */

					//printf("Sending message to socket %d\n",topdataPtr->parentSock);
					if(genericSend(topdataPtr->parentSock,&message,sizeof(message))){
						fprintf(stderr,"updateTable(): genericSend() failed for master\n");
						exit(1);
					}

				} else {
					printf("Received (%d,%c,%d), NOT forwarding to master\n",message.idnum,(char)message.antenna,message.score);

				}

				// if(!gotBoth){
				// 	if(needEnd1 && message.score){
				// 		needEnd1 = false;
				// 		end1 = message.idnum;
				// 		printf("Transmitter is near %d",end1);
				// 	}

				// 	if(!needEnd1 && !message.score){
				// 		needEnd2 = true;
				// 		printf("Transmitter is no longer near %d",end1);
				// 	}

				// 	if(needEnd2 && message.score){
				// 		needEnd2 = false;
				// 		end2 = message.idnum;
				// 		printf("Transmitter is near %d",end2);
				// 		gotBoth = true;
				// 	}
				// } else { // got both
				// 	if(message.idnum == end1 || message.idnum == end2){
				// 		printf("Should act on data\n");
				// 	}
				// }

				/* Get line in data structure */
				// if(!(alertPtr = lineLookup(topdataPtr->list,message.idnum))){
				// 	fprintf(stderr,"updateTable(): lineLookup() failed\n");
				// 	exit(1);
				// }

				/* Update the table */
				// alertPtr->metascore = recalculate(alertPtr->metascore,message.score,topdataPtr->alpha);
				// alertPtr->history[alertPtr->historyIndex++] = message.score;
				// if(alertPtr->historyIndex == TABLESIZE){
				// 	alertPtr->tableFull = true;
				// 	alertPtr->historyIndex = 0;
				// }
			}

		} // end of loop over ready sockets
	} // end of infinite loop
	return NULL; //***
}

// Example code

// A normal C function that is executed as a thread  
// when its name is specified in pthread_create() 
// void *myThreadFun(void *vargp) 
// { 
//     sleep(1); 
//     printf("Printing GeeksQuiz from Thread \n"); 
//     return NULL; 
// } 
   
// int main() 
// { 
//     pthread_t thread_id; 
//     printf("Before Thread\n"); 
//     pthread_create(&thread_id, NULL, myThreadFun, NULL); 
//     pthread_join(thread_id, NULL); 
//     printf("After Thread\n"); 
//     exit(0); 
// }

/* Never implemented. */
float recalculate(float metascore, float score, float alpha){
	return 0;
}

// int openHosts(struct Hosts hosts, int port, float timeout){
// 	for(int i = 0; i < hosts.count; i++){
// 		if((hosts.array[i].sock = openComms(hosts.array[i].addr,port,timeout)) == -1){
// 			fprintf(stderr,"openHosts(): openComms() failed for %s\n",hosts.array[i].addr);
// 			return 1;
// 		}
// 	}
// 	return 0;
// }

// int closeHosts(struct Hosts hosts){
// 	for(int i = 0; i < hosts.count; i++){
// 		if(closeComms(hosts.array[i].sock)){
// 			fprintf(stderr,"closeHosts(): closeComms() failed for %s\n",hosts.array[i].addr);
// 			return 1;
// 		}
// 	}
// 	return 0;
// }

/* If possible, sets adj1 and adj2 for endpoints */
int setAdj(struct Message *adj1, struct Message *adj2, struct Line *nodePtr1, struct Line *nodePtr2){
	char in1 = nodePtr1->triplet[0];
	// char out1 = nodePtr1->triplet[1];
	// char in2 = nodePtr2->triplet[0];
	char out2 = nodePtr2->triplet[1];
	bool got1 = false;
	bool got2 = false;

	for(int i = 0; i < nodePtr1->neighbors.count; i++){
		if(nodePtr1->neighbors.dirs[i] == in1){
			adj1->idnum = nodePtr1->neighbors.idnums[i];
			adj1->antenna = oppDir(in1);
			got1 = true;
			break;
		}
	}

	if(!got1){
		fprintf(stderr,"setAdj(): no such neighbor of end1\n");
		return 1;
	}

	for(int i = 0; i < nodePtr2->neighbors.count; i++){
		if(nodePtr2->neighbors.dirs[i] == out2){
			adj2->idnum = nodePtr2->neighbors.idnums[i];
			adj2->antenna = oppDir(out2);
			break;
			got2 = true;
		}
	}

	if(!got2){
		fprintf(stderr,"setAdj(): no such neighbor of end2\n");
		return 1;
	}


	return 0;


	// struct Line *nptr, *eptr, *sptr, *wptr;
	// nptr = eptr = sptr = wptr = NULL;
	// for(int i = 0; i < nodePtr1->neighbors.count; i++){
	// 	if(nodePtr1->neighbors.dirs[i] == 'n'){
	// 		nptr = nodePtr1->neighbors.lines[i];
	// 	} else if(nodePtr1->neighbors.dirs[i] == 'e'){
	// 		eptr = nodePtr1->neighbors.lines[i];
	// 	} else if(nodePtr1->neighbors.dirs[i] == 's'){
	// 		sptr = nodePtr1->neighbors.lines[i];
	// 	} else if(nodePtr1->neighbors.dirs[i] == 'w'){
	// 		wptr = nodePtr1->neighbors.lines[i];
	// 	}
	// }

	// // set adj1
	// if(in1 == 'n'){
	// 	if(eptr){

	// 	} else if(wptr){

	// 	} else {
	// 		printf("Edge condition\n");
	// 		return 1;
	// 	}
	// } else if(in1 = 'e'){
	// 	if(nptr){

	// 	} else if(sptr){

	// 	} else {
	// 		printf("Edge condition\n");
	// 		return 1;
	// 	}
	// } else if(in1 = 's'){
	// 	if(eptr){

	// 	} else if(wptr){

	// 	} else {
	// 		printf("Edge condition\n");
	// 		return 1;
	// 	}
	// } else if(in1 = 'w'){
	// 	if(nptr){

	// 	} else if(sptr){

	// 	} else {
	// 		printf("Edge condition\n");
	// 		return 1;
	// 	}
	// }

	// // set adj2
	// if(out2 == 'n'){

	// } else if(out2 = 'e'){

	// } else if(out2 = 's'){

	// } else if(out2 = 'w'){
		
	// }

	// return 0;
}

/* Sets path from end1 to end2, and if possible, from adj1 to adj2 */
int setpaths(struct Message2 end1, struct Message2 end2, struct Line *list){

	// prepare main path

	struct Paths allpaths;
	if(genPaths(&allpaths,list,end1.idnum,end2.idnum,true)){ // true for disjoint
		fprintf(stderr,"setpaths(): genPaths() failed for %d->%d\n",end1.idnum,end2.idnum);
		return 1;
	}
	if(!allpaths.count){
		fprintf(stderr,"setpaths(): no such paths exist for %d->%d\n",end1.idnum,end2.idnum);
		return 1;
	}
	// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
	// printPaths(stdout,links.linkset[i].allpaths);

	struct Path minpath = allpaths.paths[0];
	for(int j = 1; j < allpaths.count; j++){
		if(allpaths.paths[j].len < minpath.len){
			minpath = allpaths.paths[j];
		}
	}
	printf("Minpath from %d->%d: ",end1.idnum,end2.idnum);
	printPath(stdout,minpath);
	// for(int j = 0; j < minpath.len; j++){
	// 	minpath.nodes[j]->used = true;
	// }

	if(prepCmds(list,end1.antenna,minpath,'o')){ // 'o' for opposite
		fprintf(stderr,"setpaths(): prepCmds() failed\n");
		return 1;
	}

	// prepare adjacent path

	struct Message adj1, adj2;
	if(setAdj(&adj1,&adj2,minpath.nodes[0],minpath.nodes[minpath.len - 1])){
		fprintf(stderr,"setpaths(): setAdj failed\n");
		exit(1);
	}
	// if(setAdj(&adj2,minpath.nodes[minpath.len - 1])){
	// 	fprintf(stderr,"setpaths(): setAdj failed for adj2\n");
	// 	exit(1);
	// }
	freePaths(allpaths);
	if(genPaths(&allpaths,list,adj1.idnum,adj2.idnum,true)){ // true for disjoint
		fprintf(stderr,"setpaths(): genPaths() failed for %d->%d\n",adj1.idnum,adj2.idnum);
		return 1;
	}
	if(!allpaths.count){
		fprintf(stderr,"setpaths(): no such paths exist for %d->%d\n",adj1.idnum,adj2.idnum);
		return 1;
	}
	// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
	// printPaths(stdout,links.linkset[i].allpaths);

	minpath = allpaths.paths[0];
	for(int j = 1; j < allpaths.count; j++){
		if(allpaths.paths[j].len < minpath.len){
			minpath = allpaths.paths[j];
		}
	}
	printf("Minpath from %d->%d: ",adj1.idnum,adj2.idnum);
	printPath(stdout,minpath);
	// for(int j = 0; j < minpath.len; j++){
	// 	minpath.nodes[j]->used = true;
	// }

	if(prepCmds(list,adj1.antenna,minpath,adj2.antenna)){ // 'o' for opposite at 2
		fprintf(stderr,"setpaths(): prepCmds() failed\n");
		return 1;
	}

	freePaths(allpaths);

	// set paths

	struct Line *currlinePtr = list;
	while(currlinePtr){
		currlinePtr->used = false;
		currlinePtr = currlinePtr->next;
	}

	/* Send commands to forwarding PCs */
	printf("Sending commands to forwading PCs...\n");
	if(sendAllCmds(list)){
		fprintf(stderr,"setpaths(): sendAllCmds() failed\n");
		return 1;
	}
	return 0;
}

/* Runs in the background of the master for the single-link case.
   Reads messages as they arrive. Takes action based on giant state machine. */
void *updateMaster(void *topdataPtrVoid){
	struct Topdata *topdataPtr = (struct Topdata *)topdataPtrVoid;
	int nready, stat;
	fd_set sockSet;
	struct timeval *alertTimeout, tv;
	float alertDuration;
	// struct Host *alertPtr;
	struct Message2 message;
	// bool gotFirst = false;
	// int maxSock;

	alertTimeout = NULL;
	if(topdataPtr->alertTimeout >= 0){
		tv.tv_sec = (int)topdataPtr->alertTimeout;
		alertDuration = (topdataPtr->alertTimeout - tv.tv_sec) * 1000000;
		tv.tv_usec = (int)alertDuration;
		alertTimeout = &tv;
		// printf("%ld seconds, %ld microseconds\n",tv.tv_sec,tv.tv_usec);
	}
	// FD_ZERO(&sockSet);
	// for(int i = 0; i < topdataPtr->nhosts; i++){
	// 	FD_SET(topdataPtr->hostSocks[i],&sockSet);
	// }
	// struct Line *currlinePtr = topdataPtr->list;
	// while(currlinePtr){
	// 	if(!strcmp(currlinePtr->host->addr,topdataPtr->inet_addr)){
	// 		if(!gotFirst){
	// 			gotFirst = true;
	// 			maxSock = currlinePtr->sock;
	// 		} else {
	// 			if(currlinePtr->sock > maxSock){
	// 				maxSock = currlinePtr->sock;
	// 			}
	// 		}
	// 		// FD_SET(currlinePtr->sock,&sockSet);
	// 	}
	// 	currlinePtr = currlinePtr->next;
	// }

	// if(!gotFirst){
	// 	fprintf(stderr,"updateTable(): nothing to listen to\n");
	// 	return NULL; //***
	// }

	struct Host *currhostPtr;

	// bool gotBoth = false;
	// bool needEnd1 = true;
	// bool needEnd2 = true;
	struct Message2 end1, end2;
	end1.idnum = -1;
	end2.idnum = -1;
	int active = -1;
	// int end1 = -1;
	// char ant1, ant2;
	// int end2 = -1;
	// bool id1, id2, act1, act2;
	// id1 = id2 = act1 = act2 = false;
	// int win1index = 0;
	// int win2index = 0;
	// bool win1got2;
	// bool win2got2;
	// int win1[2], win2[2];
	// struct Line *currlinePtr;
	// char triplet[3];

	// char *line; size_t size;

	while(1){
		// printf("\t\t\tBackground\n");
		/* Detect incoming message(s) */
		FD_ZERO(&sockSet);
		currhostPtr = topdataPtr->hosts;
		while(currhostPtr){
			// if(!strcmp(currlinePtr->host->addr,topdataPtr->inet_addr)){
				// printf("Currently #%d\n",currlinePtr->idnum);
				// printf("Its sock is: %d\n",currlinePtr->sock);
				FD_SET(currhostPtr->sock,&sockSet);
			// }
			currhostPtr = currhostPtr->next;
		}

		// printf("maxHost is %d\n",topdataPtr->maxHost);
		nready = select(topdataPtr->maxHost + 1,&sockSet,NULL,NULL,alertTimeout);
		if(!nready){
			fprintf(stderr,"updateMaster(): alertTimeout expired\n");
			exit(1);
		} else if(nready == -1){
			perror("updateMaster(){select()}");
			exit(1);
		}
		// printf("There are %d ready\n",nready);

		// printf("\t\t\tBackground: nready is %d\n",nready);

		/* Process incoming messages */
		for(int i = 0; i < topdataPtr->nhosts; i++){
			if(FD_ISSET(topdataPtr->hostSocks[i],&sockSet)){
				// printf("Reading from socket %d\n",topdataPtr->hostSocks[i]);
				stat = readAnswer(topdataPtr->hostSocks[i],(uint8_t *)&message,sizeof(message));
				if(!stat){
					fprintf(stderr,"updateMaster(): readAnswer() failed to read\n");
					exit(1);
				} else if(stat == -1){
					fprintf(stderr,"updateMaster(): readAnswer() failed\n");
					exit(1);
				}

				printf("Received (%d,%c,%d)\n",message.idnum,(char)message.antenna,message.score);

				/* GIANT STATE MACHINE */
				struct Path activePath;

				if(end1.idnum == -1 && end2.idnum == -1 && active == -1){ // neither identified nor active
					if(message.score){
						printf("Identified end1\n");
						end1 = message;
						active = 1;
					}
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 1){ // neither identified, end1 active
					fprintf(stderr,"INVALID STATE: neither identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 2){ // neither identified, end2 active
					fprintf(stderr,"INVALID STATE: neither identified, end2 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == -1){ // end2 identified, neither active
					if(message.score){
						if(/* topdataPtr->near[message.idnum][end2.idnum] && */ within(&topdataPtr->timeBoundary,&end2.time,&message.time)){ // within time boundary
							printf("End2 active again\n");
							end2 = message;
							active = 2;
						} else { // outside of time boundary
							printf("Identified end1\n");
							end1 = message;
							active = 1;

							printf("Setting path end1->end2\n");
							if(setpaths2(end1,end2,&activePath,topdataPtr->list,true)){
								fprintf(stderr,"updateMaster(): setpaths2() failed\n");
								exit(1);
							}
							// printf("HALTING, ENTER a TO ABORT\n");
							// line = NULL; size = 0;
							// getline(&line,&size,stdin);
							// while(line[0] != 'a'){
							// 	free(line);
							// 	line = NULL;
							// 	size = 0;
							// 	getline(&line,&size,stdin);
							// }
							// free(line);

							// printf("Setting all to e-0\n");
							// struct Line *currlinePtr = topdataPtr->list;
							// char triplet[3];
							// while(currlinePtr){
							// 	setTriplet(triplet,'e','-','0');
							// 	if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,triplet)){
							// 		fprintf(stderr,"updateMaster(): sendQuad() failed\n");
							// 		exit(1);
							// 	}
							// 	currlinePtr = currlinePtr->next;
							// }

							// exit(1);
						}
					}
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 1){ // end2 identified, end1 active
					fprintf(stderr,"INVALID STATE: end2 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 2){ // end2 identified, end2 active
					if(message.idnum == end2.idnum){ // got update from end2
						if(!message.score){
							printf("End2 no longer active\n");
							active = -1;
						} else {
							if(message.score > end2.score){
								// printf("Reinforcing end2\n");
								// end2 = message;
							}
						}
					} else { // received from another unit
						if(/* topdataPtr->near[message.idnum][end2.idnum] && */ within(&topdataPtr->timeBoundary,&end2.time,&message.time)){ // within time boundary
							if(message.score > end2.score){
								printf("Replacing end2 with neighbor\n");
								end2 = message;
							}
						} else { // outside of neighborhood
							if(message.score){
								printf("Identified end1\n");
								end1 = message;
								active = 1;

								printf("Setting path end1->end2\n");
								if(setpaths2(end1,end2,&activePath,topdataPtr->list,true)){
									fprintf(stderr,"updateMaster(): setpaths2() failed\n");
									exit(1);
								}
								// printf("HALTING, ENTER a TO ABORT\n");
								// line = NULL; size = 0;
								// getline(&line,&size,stdin);
								// while(line[0] != 'a'){
								// 	free(line);
								// 	line = NULL;
								// 	size = 0;
								// 	getline(&line,&size,stdin);
								// }
								// free(line);

								// printf("Setting all to e-0\n");
								// struct Line *currlinePtr = topdataPtr->list;
								// char triplet[3];
								// while(currlinePtr){
								// 	setTriplet(triplet,'e','-','0');
								// 	if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,triplet)){
								// 		fprintf(stderr,"updateMaster(): sendQuad() failed\n");
								// 		exit(1);
								// 	}
								// 	currlinePtr = currlinePtr->next;
								// }

								// exit(1);
							}
						}
					}
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == -1){ // end1 identified, neither active
					if(message.score){
						if(topdataPtr->near[message.idnum][end1.idnum] /*&& within(&topdataPtr->timeBoundary,&end1.time,&message.time)*/){ // within neighborhood
							printf("End1 active again\n");
							end1 = message;
							active = 1;
						} else { // outside of neighborhood
							printf("Identified end2\n");
							end2 = message;
							active = 2;

							printf("Setting path end1->end2\n");
							if(setpaths2(end1,end2,&activePath,topdataPtr->list,true)){
								fprintf(stderr,"updateMaster(): setpaths2() failed\n");
								exit(1);
							}
							// printf("HALTING, ENTER a TO ABORT\n");
							// line = NULL; size = 0;
							// getline(&line,&size,stdin);
							// while(line[0] != 'a'){
							// 	free(line);
							// 	line = NULL;
							// 	size = 0;
							// 	getline(&line,&size,stdin);
							// }
							// free(line);

							// printf("Setting all to e-0\n");
							// struct Line *currlinePtr = topdataPtr->list;
							// char triplet[3];
							// while(currlinePtr){
							// 	setTriplet(triplet,'e','-','0');
							// 	if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,triplet)){
							// 		fprintf(stderr,"updateMaster(): sendQuad() failed\n");
							// 		exit(1);
							// 	}
							// 	currlinePtr = currlinePtr->next;
							// }

							// exit(1);
						}
					}
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 1){ // end1 identified, end1 active
					if(message.idnum == end1.idnum){ // got update from end1
						if(!message.score){
							printf("End1 no longer active\n");
							active = -1;
						} else {
							if(message.score > end1.score){
								// printf("Reinforcing end1\n");
								// end1 = message;
							}
						}
					} else { // received from another unit
						if(/* topdataPtr->near[message.idnum][end1.idnum] && */ within(&topdataPtr->timeBoundary,&end1.time,&message.time)){ // within neighborhood
							if(message.score > end1.score){
								printf("Replacing end1 with neighbor\n");
								end1 = message;
							}
						} else { // outside of neighborhood
							if(message.score){
								printf("Identified end2\n");
								end2 = message;
								active = 2;

								printf("Setting path end1->end2\n");
								if(setpaths2(end1,end2,&activePath,topdataPtr->list,true)){
									fprintf(stderr,"updateMaster(): setpaths2() failed\n");
									exit(1);
								}
								// printf("HALTING, ENTER a TO ABORT\n");
								// line = NULL; size = 0;
								// getline(&line,&size,stdin);
								// while(line[0] != 'a'){
								// 	free(line);
								// 	line = NULL;
								// 	size = 0;
								// 	getline(&line,&size,stdin);
								// }
								// free(line);

								// printf("Setting all to e-0\n");
								// struct Line *currlinePtr = topdataPtr->list;
								// char triplet[3];
								// while(currlinePtr){
								// 	setTriplet(triplet,'e','-','0');
								// 	if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,triplet)){
								// 		fprintf(stderr,"updateMaster(): sendQuad() failed\n");
								// 		exit(1);
								// 	}
								// 	currlinePtr = currlinePtr->next;
								// }

								// exit(1);
							}
						}
					}
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 2){ // end1 identified, end2 active
					fprintf(stderr,"INVALID STATE: end1 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == -1){ // both identified, neither active
					if(message.score){
						if(message.idnum == end1.idnum){
							active = 1;
							printf("Setting path end1->end2\n");
							if(setpaths2(end1,end2,&activePath,topdataPtr->list,true)){
								fprintf(stderr,"updateMaster(): setpaths2() failed\n");
								exit(1);
							}
						} else if(message.idnum == end2.idnum){
							active = 2;
							printf("Setting path end2->end1\n");
							if(setpaths2(end2,end1,&activePath,topdataPtr->list,true)){
								fprintf(stderr,"updateMaster(): setpaths2() failed\n");
								exit(1);
							}
						} else {
							printf("Ignoring message from %d\n",message.idnum);
						}
					}
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 1){ // both identified, end1 active
					if(message.idnum == end1.idnum){
						if(!message.score){
							printf("End1 no longer active\n");
							active = -1;
							printf("Unsetting path\n");
							if(unsetPath(activePath)){
								fprintf(stderr,"unsetPath() failed\n");
								exit(1);
							}
						}
					} else if(message.idnum == end2.idnum){
						if(message.score){
							printf("End2 now active\n");
							active = 2;
							printf("Reversing path\n");
							if(reversePath(activePath)){
								fprintf(stderr,"reversePath() failed\n");
								exit(1);
							}
						}
					} else {
						printf("Ignoring message from %d\n",message.idnum);
					}
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 2){ // both identified, end2 active
					if(message.idnum == end2.idnum){
						if(!message.score){
							printf("End2 no longer active\n");
							active = -1;
							printf("Unsetting path\n");
							if(unsetPath(activePath)){
								fprintf(stderr,"unsetPath() failed\n");
								exit(1);
							}
						}
					} else if(message.idnum == end1.idnum){
						if(message.score){
							printf("End1 now active\n");
							active = 1;
							printf("Reversing path\n");
							if(reversePath(activePath)){
								fprintf(stderr,"reversePath() failed\n");
								exit(1);
							}
						}
					} else {
						printf("Ignoring message from %d\n",message.idnum);
					}
				} else {
					fprintf(stderr,"INVALID STATE: else case; end1 is %d, end2 is %d, active is %d\n",end1.idnum,end2.idnum,active);
					exit(1);
				}






				// if(end1.idnum == -1 && end2.idnum == -1){ // neither identified
				// 	if(message.score){
				// 		printf("Identified end1\n"):
				// 		end1 = message;
				// 		active = 1;
				// 	}
				// } else if(end1.idnum != -1 && end2.idnum == -1){ // only end1 identified
				// 	if(message.idnum = end1.idnum){ // got update from end1
				// 		if(!message.score){
				// 			printf("End1 no longer active\n");
				// 			// end1 = -1;
				// 			active = 0;
				// 		} else if(message.score > end1.score){
				// 			printf("Reinforcing end1\n");
				// 			end1 = message;
				// 		}
				// 	} else { // got report from another unit
				// 		if(topdata->near[end1.idnum][message.idnum]){ // within neighborhood of end1
				// 			if(message.score > end1.score){
				// 				printf("Replacing end1\n");
				// 				end1 = message;
				// 			}
				// 		} else { // outside of neighborhood of end1
				// 			if(message.score){
				// 				printf("Identified end2\n");
				// 				end2 = message;
				// 				active = 2;
				// 			}
				// 		}
				// 	}
				// } else if(end1.idnum == -1 && end2.idnum != -1){ // only end2 identified
				// 	if(message.idnum = end2.idnum){ // got update from end2
				// 		if(!message.score){
				// 			printf("Lost end2\n");
				// 			end2 = -1;
				// 		} else if(message.score > end2.score){
				// 			printf("Reinforcing end2\n");
				// 			end2 = message;
				// 		}
				// 	} else { // got report from another unit
				// 		if(topdata->near[end2.idnum][message.idnum]){ // within neighborhood of end2
				// 			if(message.score > end2.score){
				// 				printf("Replacing end2\n");
				// 				end2 = message;
				// 			}
				// 		} else { // outside of neighborhood of end2
				// 			if(message.score){
				// 				printf("Identified end1\n");
				// 				end1 = message;
				// 			}
				// 		}
				// 	}
				// } else if(end1.idnum != -1 && end2.idnum != -1){ // both identified

				// } else {
				// 	fprintf(stderr,"INVALID STATE\n");
				// 	exit(1);
				// }

				// if(end1.idnum == -1){ // need an end1
				// 	if(message.score){
				// 		printf("Found end1\n");
				// 		end1 = message;
				// 	}
				// } else { // have an end1
				// 	if(topdata->near[end1.idnum][message.idnum]){ // received from neighborhood
				// 		if(message.score > end1.score){
				// 			printf("Replacing end1\n");
				// 			end1 = message;
				// 		}
				// 	} else { // received from outside of neighborhood
				// 		if(message.score){
				// 			printf("Found end2\n");
				// 			end2 = message;
				// 		}
				// 	}
				// }

				/* GIANT STATE MACHINE */

				// if(id1 && id2 && act1 && act2){ // both id'd, both active
				// 	printf("UNDEFINED: Can't both be active\n");
				// } else if(id1 && id2 && act1 && !act2){ // both id'd, end1 active
				// 	if(message.idnum == end1){
				// 		if(message.score){
				// 			// printf("End1 still active\n");
				// 		} else {
				// 			printf("End1 no longer active\n");
				// 			act1 = false;
				// 			printf("Should unset path\n");

				// 			currlinePtr = topdataPtr->list;
				// 			while(currlinePtr){
				// 				setTriplet(triplet,'e','-','0');
				// 				if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,triplet)){
				// 					fprintf(stderr,"updateMaster(): sendQuad() failed\n");
				// 					exit(1);
				// 				}
				// 				currlinePtr = currlinePtr->next;
				// 			}
				// 		}
				// 	} else if(message.idnum == end2){
				// 		if(message.score){
				// 			printf("End2 replacing end1 as active\n");
				// 			act2 = true;
				// 			act1 = false;
				// 		}
				// 	} else {
				// 		printf("Ignoring message from other source\n");
				// 	}
				// } else if(id1 && id2 && !act1 && act2){ // both id'd, end2 active
				// 	if(message.idnum == end1){
				// 		if(message.score){
				// 			printf("End1 replacing end2 as active\n");
				// 			act1 = true;
				// 			act2 = false;
				// 		}
				// 	} else if(message.idnum == end2){
				// 		if(message.score){
				// 			// printf("End2 still active\n");
				// 		} else {
				// 			printf("End2 no longer active\n");
				// 			act2 = false;
				// 			printf("Should unset path\n");

				// 			currlinePtr = topdataPtr->list;
				// 			while(currlinePtr){
				// 				setTriplet(triplet,'e','-','0');
				// 				if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,triplet)){
				// 					fprintf(stderr,"updateMaster(): sendQuad() failed\n");
				// 					exit(1);
				// 				}
				// 				currlinePtr = currlinePtr->next;
				// 			}
				// 		}
				// 	} else {
				// 		printf("Ignoring message from other source\n");
				// 	}
				// } else if(id1 && id2 && !act1 && !act2){ // both id'd, neither active
				// 	// if(message.idnum == end1){
				// 	// 	if(message.score){
				// 	// 		printf("End1 now active\n");
				// 	// 		act1 = true;
				// 	// 	}
				// 	// } else if(message.idnum == end2){
				// 	// 	if(message.score){
				// 	// 		printf("End2 now active\n");
				// 	// 		act2 = true;
				// 	// 	}
				// 	// } else {
				// 	// 	printf("Ignoring message from other source\n");
				// 	// }
				// } else if(id1 && !id2 && act1 && act2){ // end1 id'd, both active
				// 	printf("UNDEFINED: Can't both be active\n");
				// } else if(id1 && !id2 && act1 && !act2){ // end1 id'd, end1 active
				// 	if(message.idnum == end1){
				// 		if(!message.score){
				// 			printf("End1 no longer active\n");
				// 			act1 = false;
				// 		}
				// 	} else {
				// 		printf("For debugging:\n");
				// 		printf("message.score is: %d\n",message.score);
				// 		printf("message.idnum is: %d\n",message.idnum);
				// 		printf("end1 is: %d\n",end1);
				// 		if(message.score){
				// 			printf("topdataPtr->near[message.idnum][end1] is: %d\n",topdataPtr->near[message.idnum][end1]);
				// 		}
				// 		printf("Herein not debugging\n");
				// 		if(message.score && !topdataPtr->near[message.idnum][end1]){
				// 			printf("End2 id'd, replacing end1 as active\n");
				// 			end2 = message.idnum;
				// 			id2 = true;
				// 			act2 = true;
				// 			act1 = false;
				// 			printf("Should set path\n");
							
				// 			struct Paths allpaths;
				// 			if(genPaths(&allpaths,topdataPtr->list,end1,end2,false)){ // true for disjoint
				// 				fprintf(stderr,"updateMaster(): genPaths() failed for %d->%d\n",end1,end2);
				// 				exit(1);
				// 			}
				// 			if(!allpaths.count){
				// 				fprintf(stderr,"updateMaster(): no such paths exist for %d->%d\n",end1,end2);
				// 				exit(1);
				// 			}
				// 			// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
				// 			// printPaths(stdout,links.linkset[i].allpaths);

				// 			struct Path minpath = allpaths.paths[0];
				// 			for(int j = 1; j < allpaths.count; j++){
				// 				if(allpaths.paths[j].len < minpath.len){
				// 					minpath = allpaths.paths[j];
				// 				}
				// 			}
				// 			printf("Minpath from %d->%d: ",end1,end2);
				// 			printPath(stdout,minpath);
				// 			// for(int j = 0; j < minpath.len; j++){
				// 			// 	minpath.nodes[j]->used = true;
				// 			// }

				// 			if(prepCmds(topdataPtr->list,'-',minpath,'-')){
				// 				fprintf(stderr,"updateMaster(): prepCmds() failed\n");
				// 				exit(1);
				// 			}

				// 			freePaths(allpaths);

				// 			// currlinePtr = topdata.list;
				// 			// while(currlinePtr){
				// 			// 	currlinePtr->used = false;
				// 			// 	currlinePtr = currlinePtr->next;
				// 			// }

				// 			/* Send commands to forwarding PCs */
				// 			printf("Sending commands to forwading PCs...\n");
				// 			if(sendAllCmds(topdataPtr->list)){
				// 				fprintf(stderr,"setpath: sendAllCmds() failed\n");
				// 				exit(1);
				// 			}

				// 			printf("PATH IS SET, PRESS ENTER TO ABORT\n");
				// 			getchar();
				// 			exit(1);
				// 		}
				// 	}
				// } else if(id1 && !id2 && !act1 && act2){ // end1 id'd, end2 active
				// 	printf("UNDEFINED: Can't be active if not id'd\n");
				// } else if(id1 && !id2 && !act1 && !act2){ // end1 id'd, neither active
				// 	if(message.idnum == end1){
				// 		if(message.score){
				// 			printf("End1 now active\n");
				// 			act1 = true;
				// 		}
				// 	} else {
				// 		if(message.score && !topdataPtr->near[message.idnum][end1]){
				// 			printf("End2 id'd, now active\n");
				// 			end2 = message.idnum;
				// 			id2 = true;
				// 			act2 = true;
				// 			printf("Should set path\n");

				// 			struct Paths allpaths;
				// 			if(genPaths(&allpaths,topdataPtr->list,end1,end2,false)){ // true for disjoint
				// 				fprintf(stderr,"updateMaster(): genPaths() failed for %d->%d\n",end1,end2);
				// 				exit(1);
				// 			}
				// 			if(!allpaths.count){
				// 				fprintf(stderr,"updateMaster(): no such paths exist for %d->%d\n",end1,end2);
				// 				exit(1);
				// 			}
				// 			// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
				// 			// printPaths(stdout,links.linkset[i].allpaths);

				// 			struct Path minpath = allpaths.paths[0];
				// 			for(int j = 1; j < allpaths.count; j++){
				// 				if(allpaths.paths[j].len < minpath.len){
				// 					minpath = allpaths.paths[j];
				// 				}
				// 			}
				// 			printf("Minpath from %d->%d: ",end1,end2);
				// 			printPath(stdout,minpath);
				// 			// for(int j = 0; j < minpath.len; j++){
				// 			// 	minpath.nodes[j]->used = true;
				// 			// }

				// 			if(prepCmds(topdataPtr->list,'-',minpath,'-')){
				// 				fprintf(stderr,"updateMaster(): prepCmds() failed\n");
				// 				exit(1);
				// 			}

				// 			freePaths(allpaths);

				// 			// currlinePtr = topdata.list;
				// 			// while(currlinePtr){
				// 			// 	currlinePtr->used = false;
				// 			// 	currlinePtr = currlinePtr->next;
				// 			// }

				// 			/* Send commands to forwarding PCs */
				// 			printf("Sending commands to forwading PCs...\n");
				// 			if(sendAllCmds(topdataPtr->list)){
				// 				fprintf(stderr,"setpath: sendAllCmds() failed\n");
				// 				exit(1);
				// 			}

				// 			printf("PATH IS SET, PRESS ENTER TO ABORT\n");
				// 			getchar();
				// 			exit(1);
				// 		}
				// 	}
				// } else if(!id1 && id2 && act1 && act2){ // end2 id'd, both active
				// 	printf("UNDEFINED: Can't both be active\n");
				// } else if(!id1 && id2 && act1 && !act2){ // end2 id'd, end1 active
				// 	printf("UNDEFINED: Can't be active if not id'd\n");
				// } else if(!id1 && id2 && !act1 && act2){ // end2 id'd, end2 active
				// 		if(message.idnum == end2){
				// 			if(!message.score){
				// 				printf("End2 no longer active\n");
				// 				act2 = false;
				// 			}
				// 		} else { // not from end2
				// 			if(message.score && !topdataPtr->near[message.idnum][end2]){
				// 				printf("End1 id'd, replacing end2 as active\n");
				// 				end1 = message.idnum;
				// 				id1 = true;
				// 				act1 = true;
				// 				act2 = false;
				// 				printf("Should set path\n");

				// 				struct Paths allpaths;
				// 				if(genPaths(&allpaths,topdataPtr->list,end1,end2,false)){ // true for disjoint
				// 					fprintf(stderr,"updateMaster(): genPaths() failed for %d->%d\n",end1,end2);
				// 					exit(1);
				// 				}
				// 				if(!allpaths.count){
				// 					fprintf(stderr,"updateMaster(): no such paths exist for %d->%d\n",end1,end2);
				// 					exit(1);
				// 				}
				// 				// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
				// 				// printPaths(stdout,links.linkset[i].allpaths);

				// 				struct Path minpath = allpaths.paths[0];
				// 				for(int j = 1; j < allpaths.count; j++){
				// 					if(allpaths.paths[j].len < minpath.len){
				// 						minpath = allpaths.paths[j];
				// 					}
				// 				}
				// 				printf("Minpath from %d->%d: ",end1,end2);
				// 				printPath(stdout,minpath);
				// 				// for(int j = 0; j < minpath.len; j++){
				// 				// 	minpath.nodes[j]->used = true;
				// 				// }

				// 				if(prepCmds(topdataPtr->list,'-',minpath,'-')){
				// 					fprintf(stderr,"updateMaster(): prepCmds() failed\n");
				// 					exit(1);
				// 				}

				// 				freePaths(allpaths);

				// 				// currlinePtr = topdata.list;
				// 				// while(currlinePtr){
				// 				// 	currlinePtr->used = false;
				// 				// 	currlinePtr = currlinePtr->next;
				// 				// }

				// 				/* Send commands to forwarding PCs */
				// 				printf("Sending commands to forwading PCs...\n");
				// 				if(sendAllCmds(topdataPtr->list)){
				// 					fprintf(stderr,"setpath: sendAllCmds() failed\n");
				// 					exit(1);
				// 				}

				// 				printf("PATH IS SET, PRESS ENTER TO ABORT\n");
				// 				getchar();
				// 				exit(1);
				// 			}
				// 		}
				// } else if(!id1 && id2 && !act1 && !act2){ // end2 id'd, neither active
				// 	if(message.idnum == end2){
				// 		if(message.score){
				// 			printf("End2 now active\n");
				// 			act2 = true;
				// 		}
				// 	} else {
				// 		if(message.score && !topdataPtr->near[message.idnum][end2]){
				// 			printf("End1 id'd, now active\n");
				// 			end1 = message.idnum;
				// 			id1 = true;
				// 			act1 = true;
				// 			printf("Should set path\n");

				// 			struct Paths allpaths;
				// 			if(genPaths(&allpaths,topdataPtr->list,end1,end2,false)){ // true for disjoint
				// 				fprintf(stderr,"updateMaster(): genPaths() failed for %d->%d\n",end1,end2);
				// 				exit(1);
				// 			}
				// 			if(!allpaths.count){
				// 				fprintf(stderr,"updateMaster(): no such paths exist for %d->%d\n",end1,end2);
				// 				exit(1);
				// 			}
				// 			// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
				// 			// printPaths(stdout,links.linkset[i].allpaths);

				// 			struct Path minpath = allpaths.paths[0];
				// 			for(int j = 1; j < allpaths.count; j++){
				// 				if(allpaths.paths[j].len < minpath.len){
				// 					minpath = allpaths.paths[j];
				// 				}
				// 			}
				// 			printf("Minpath from %d->%d: ",end1,end2);
				// 			printPath(stdout,minpath);
				// 			// for(int j = 0; j < minpath.len; j++){
				// 			// 	minpath.nodes[j]->used = true;
				// 			// }

				// 			if(prepCmds(topdataPtr->list,'-',minpath,'-')){
				// 				fprintf(stderr,"updateMaster(): prepCmds() failed\n");
				// 				exit(1);
				// 			}

				// 			freePaths(allpaths);

				// 			// currlinePtr = topdata.list;
				// 			// while(currlinePtr){
				// 			// 	currlinePtr->used = false;
				// 			// 	currlinePtr = currlinePtr->next;
				// 			// }

				// 			/* Send commands to forwarding PCs */
				// 			printf("Sending commands to forwading PCs...\n");
				// 			if(sendAllCmds(topdataPtr->list)){
				// 				fprintf(stderr,"setpath: sendAllCmds() failed\n");
				// 				exit(1);
				// 			}

				// 			printf("PATH IS SET, PRESS ENTER TO ABORT\n");
				// 			getchar();
				// 			exit(1);
				// 		}
				// 	}
				// } else if(!id1 && !id2 && act1 && act2){ // neither id'd, both active
				// 	printf("UNDEFINED: Can't be active if not id'd, and can't both be active\n");
				// } else if(!id1 && !id2 && act1 && !act2){ // neither id'd, end1 active
				// 	printf("UNDEFINED: Can't be active if not id'd\n");
				// } else if(!id1 && !id2 && !act1 && act2){ // neither id'd, end2 active
				// 	printf("UNDEFINED: Can't be active if not id'd\n");
				// } else if(!id1 && !id2 && !act1 && !act2){ // neither id'd, neither active; initial state
				// 	if(message.score){
				// 		printf("End1 id'd, now active\n");
				// 		end1 = message.idnum;
				// 		id1 = true;
				// 		act1 = true;
				// 	}
				// }

				// if(needEnd1 && needEnd2){ // have neither
				// 	if(message.score){
				// 		end1 = message.idnum;
				// 		printf("Transmitter is near end1: %d\n",end1);
				// 		needEnd1 = false;
				// 	}
				// } else if(needEnd1 && !needEnd2){ // only have end2
				// 	if(message.idnum == end2){
				// 		// data or "lost" message
				// 		if(!message.score){
				// 			printf("Lost end2\n");
				// 			needEnd2 = true;
				// 		} else {
				// 			printf("Act on data\n");
				// 		}
				// 	} else { // not from end2
				// 		if(message.score){
				// 			end1 = message.idnum;
				// 			printf("Transmitter is near end1: %d\n",end1);
				// 			needEnd1 = false;
				// 			printf("Should set path\n");
				// 		}
				// 	}
				// } else if(!needEnd1 && needEnd2){ // only have end1
				// 	if(message.idnum == end1){
				// 		// data or "lost" message
				// 		if(!message.score){
				// 			printf("Lost end1\n");
				// 			needEnd1 = true;
				// 		} else {
				// 			printf("Act on data\n");
				// 		}
				// 	} else { // not from end1
				// 		if(message.score){
				// 			end2 = message.idnum;
				// 			printf("Transmitter is near end2: %d\n",end2);
				// 			needEnd2 = false;
				// 			printf("Should set path\n");
				// 		}
				// 	}
				// } else { // have both
				// 	if(message.idnum == end1){
				// 		// data or "lost" message
				// 		if(!message.score){
				// 			printf("Lost end1\n");
				// 			needEnd1 = true;
				// 		} else {
				// 			printf("Act on data\n");
				// 		}
				// 	} else if(message.idnum == end2){
				// 		// data or "lost" message
				// 		if(!message.score){
				// 			printf("Lost end2\n");
				// 			needEnd2 = true;
				// 		} else {
				// 			printf("Act on data\n");
				// 		}
				// 	} else { // not from end1 or end2
				// 		// ignore
				// 	}
				// }


				// if(needEnd1 || needEnd2){
				// 	if(needEnd1 && message.score){
				// 		if(needEnd2){
				// 			end1 = message.idnum;
				// 			printf("Transmitter is near end1: %d\n",end1);
				// 			needEnd1 = false;
				// 			win1got2 = false;
				// 		} else {
				// 			if(message.idnum != end2){
				// 				end1 = message.idnum;
				// 				printf("Transmitter is near end1: %d\n",end1);
				// 				needEnd1 = false;
				// 				win1got2 = false;
				// 			}
				// 			printf("Should set path\n");
				// 		}
				// 	}

				// 	if(needEnd2 && message.score){
				// 		if(needEnd1){
				// 			end2 = message.idnum;
				// 			printf("Transmitter is near end2: %d\n",end2);
				// 			needEnd2 = false;
				// 			win2got2 = false;
				// 		} else {
				// 			if(message.idnum != end1){
				// 				end2 = message.idnum;
				// 				printf("Transmitter is near end2: %d\n",end2);
				// 				needEnd2 = false;
				// 				win2got2 = false;
				// 			}
				// 			printf("Should set path\n");
				// 		}
				// 	}

				// 	if(!needEnd1 && !message.score && win1got2 && !win1[0] && !win1[1] && message.idnum == end1){
				// 		printf("Lost end1\n");
				// 		needEnd1 = true;
				// 	}

				// 	if(!needEnd2 && !message.score && win2got2 && !win2[0] && !win2[1] && message.idnum == end2){
				// 		printf("Lost end2\n");
				// 		needEnd2 = true;
				// 	}
				// } else { // got both
				// 	if(message.idnum == end1){
				// 		if(!message.score && win1got2 && !win1[0] && !win1[1]){
				// 			printf("Lost end1\n");
				// 			printf("Should unset path\n");
				// 			needEnd1 = true;
				// 			continue;
				// 		}
				// 		win1[win1index] = message.score;
				// 		if(win1index){
				// 			win1got2 = true;
				// 		}
				// 		win1index = !win1index;

				// 		printf("Should act on information\n");
				// 	}

				// 	if(message.idnum == end2){
				// 		if(!message.score && win2got2 && !win2[0] && !win1[1]){
				// 			printf("Lost end2\n");
				// 			printf("Should unset path\n");
				// 			needEnd2 = true;
				// 			continue;
				// 		}
				// 		win2[win2index] = message.score;
				// 		if(win2index){
				// 			win2got2 = true;
				// 		}
				// 		win2index = !win2index;

				// 		printf("Should act on information\n");
				// 	}

					// if(message.idnum == end1 || message.idnum == end2){
					// 	printf("Should act on data\n");
					// }
				// }

				/* Get line in data structure */
				// if(!(alertPtr = lineLookup(topdataPtr->list,message.idnum))){
				// 	fprintf(stderr,"updateTable(): lineLookup() failed\n");
				// 	exit(1);
				// }

				/* Update the table */
				// alertPtr->history[alertPtr->historyIndex++] = message.score;
				// if(alertPtr->historyIndex == TABLESIZE){
				// 	alertPtr->tableFull = true;
				// 	alertPtr->historyIndex = 0;
				// }

			}


		} // end of loop over ready sockets
	} // end of infinite loop
	return NULL; //***
}

/* Returns float version of a struct timeval, in seconds */
float floatTV(struct timeval tv){
	return (float)tv.tv_sec + ((float)tv.tv_usec)/1000000.0;
}

/* Check if difference in timevals is less than the time boundary */
bool within(struct timeval *timeBoundaryPtr, struct timeval *firstTimePtr, struct timeval *currTimePtr){
	// printf("timeBoundary: %f, firstTime: %f, currTime: %f\n",floatTV(*timeBoundaryPtr),floatTV(*firstTimePtr),floatTV(*currTimePtr));
	struct timeval diff;
	timersub(currTimePtr,firstTimePtr,&diff);
	printf("diff: %f\n",floatTV(diff));
	if(timercmp(&diff,timeBoundaryPtr, < )){ // within boundary
		printf("Within boundary? True\n");
		return true;
	} else {
		printf("Within boundary? False\n");
		return false;
	}
}


int unsetPath(struct Path path){
	for(int i = 0; i < path.len; i++){
		printf("Node %d currently %c%c%c, now setting to e-0\n",path.nodes[i]->idnum,
			path.nodes[i]->triplet[0],path.nodes[i]->triplet[1],path.nodes[i]->triplet[2]);
		if(sendQuad(path.nodes[i]->host->sock,path.nodes[i]->idnum,(char *)"e-0")){
			fprintf(stderr,"unsetPath(): sendQuad() failed\n");
			return 1;
		}
	}
	return 0;
}
int reversePath(struct Path path){
	for(int i = 0; i < path.len; i++){
		printf("Node %d currently %c%c%c, now setting to %c%c%c\n",path.nodes[i]->idnum,
			path.nodes[i]->triplet[0],path.nodes[i]->triplet[1],path.nodes[i]->triplet[2],
			path.nodes[i]->triplet[1],path.nodes[i]->triplet[0],path.nodes[i]->triplet[2]);
		setTriplet(path.nodes[i]->triplet,path.nodes[i]->triplet[1],path.nodes[i]->triplet[0],path.nodes[i]->triplet[2]);
		if(sendQuad(path.nodes[i]->host->sock,path.nodes[i]->idnum,path.nodes[i]->triplet)){
			fprintf(stderr,"unsetPath(): sendQuad() failed\n");
			return 1;
		}
	}
	return 0;
}

int assignToPair(struct Message2 message, struct Pair *pairs, int *numPairs, struct Pair **lastAssigned, struct Topdata *topdataPtr){
	struct Pair *currpairPtr = pairs, *prevpairPtr = NULL;
	while(currpairPtr){
		if(currpairPtr->numIDed == 1){
			currpairPtr->numIDed++;
			currpairPtr->active = 2;
			currpairPtr->end2 = message;
			*lastAssigned = currpairPtr;
			printf("Setting path from %d to %d\n",currpairPtr->end2.idnum,currpairPtr->end1.idnum);
			if(setpaths2(currpairPtr->end2,currpairPtr->end1,&currpairPtr->path,topdataPtr->list,true)){ // true for "first path"
				fprintf(stderr,"updateConc(): setpaths2() failed\n");
				exit(1);
			}
			return 0;
		}
		prevpairPtr = currpairPtr;
		currpairPtr = currpairPtr->next;
	}
	// no partial pairs
	*lastAssigned = prevpairPtr->next = malloc(sizeof(*prevpairPtr->next));
	if(!*lastAssigned){
		fprintf(stderr,"assignToPair(): malloc() failed\n");
		return 1;
	}
	prevpairPtr->next->prev = prevpairPtr;
	currpairPtr = prevpairPtr->next;
	currpairPtr->numIDed = 1;
	currpairPtr->active = 1;
	currpairPtr->end1 = message;
	currpairPtr->next = NULL;
	return 0;
}

// Example code

// A normal C function that is executed as a thread  
// when its name is specified in pthread_create() 
// void *myThreadFun(void *vargp) 
// { 
//     sleep(1); 
//     printf("Printing GeeksQuiz from Thread \n"); 
//     return NULL; 
// } 
   
// int main() 
// { 
//     pthread_t thread_id; 
//     printf("Before Thread\n"); 
//     pthread_create(&thread_id, NULL, myThreadFun, NULL); 
//     pthread_join(thread_id, NULL); 
//     printf("After Thread\n"); 
//     exit(0); 
// }


/* Runs in the background of the master for the concurrent-link case.
   Reads messages as they arrive. Takes action based on giant state machine. */
void *updateConc(void *topdataPtrVoid){
	struct Topdata *topdataPtr = (struct Topdata *)topdataPtrVoid;
	int nready, stat;
	fd_set sockSet;
	struct timeval *alertTimeout, tv;
	float alertDuration;
	struct Message2 message;

	alertTimeout = NULL;
	if(topdataPtr->alertTimeout >= 0){
		tv.tv_sec = (int)topdataPtr->alertTimeout;
		alertDuration = (topdataPtr->alertTimeout - tv.tv_sec) * 1000000;
		tv.tv_usec = (int)alertDuration;
		alertTimeout = &tv;
	}

	struct Host *currhostPtr;
	// struct Message2 end1, end2, end3, end4;
	// end1.idnum = -1;
	// end2.idnum = -1;
	// end3.idnum = -1;
	// end4.idnum = -1;
	// int active = -1;
	// struct Pair pair1, pair2;
	// pair1.numIDed = 0;
	// pair1.end1.idnum = -1;
	// pair1.end2.idnum = -1;
	// pair1.active = -1;
	// pair2.numIDed = 0;
	// pair2.end1.idnum = -1;
	// pair2.end2.idnum = -1;
	// pair2.active = -1;
	// char *line; size_t size;

	struct Pair *pairs = NULL, *lastAssigned = NULL;
	int numPairs = 0;

	// int *endIDs = NULL;
	// int numEnds = 0;

	while(1){

		/* Detect incoming message(s) */
		FD_ZERO(&sockSet);
		currhostPtr = topdataPtr->hosts;
		while(currhostPtr){
			FD_SET(currhostPtr->sock,&sockSet);
			currhostPtr = currhostPtr->next;
		}

		// printf("maxHost is %d\n",topdataPtr->maxHost);
		nready = select(topdataPtr->maxHost + 1,&sockSet,NULL,NULL,alertTimeout);
		if(!nready){
			fprintf(stderr,"updateMaster(): alertTimeout expired\n");
			exit(1);
		} else if(nready == -1){
			perror("updateMaster(){select()}");
			exit(1);
		}
		// printf("There are %d ready\n",nready);

		/* Process incoming messages */
		for(int i = 0; i < topdataPtr->nhosts; i++){
			if(FD_ISSET(topdataPtr->hostSocks[i],&sockSet)){
				// printf("Reading from socket %d\n",topdataPtr->hostSocks[i]);
				stat = readAnswer(topdataPtr->hostSocks[i],(uint8_t *)&message,sizeof(message));
				if(!stat){
					fprintf(stderr,"updateConc(): readAnswer() failed to read\n");
					exit(1);
				} else if(stat == -1){
					fprintf(stderr,"updateConc(): readAnswer() failed\n");
					exit(1);
				}

				printf("Received (%d,%c,%d)\n",message.idnum,(char)message.antenna,message.score);

				if(numPairs == 0){
					if(message.score){
						printf("IDed first end\n");
						numPairs++;
						pairs = malloc(sizeof(*pairs));
						if(!pairs){
							fprintf(stderr,"malloc() failed\n");
							exit(1);
						}
						pairs->end1 = message;
						pairs->active = 1;
						pairs->numIDed = 1;
						pairs->next = NULL;
						pairs->prev = pairs;
						lastAssigned = pairs;
					}
				} else { // at least one end previously found

					// check if update
					bool gotUpdate = false;
					struct Pair *currPair = pairs;
					while(currPair){
					// for(int i = 0; i < numPairs; i++){
						if(currPair->end1.idnum == message.idnum ||
							(currPair->numIDed == 2 && currPair->end2.idnum == message.idnum)){
							gotUpdate = true;
							// Want:
							// If inactive becomes active, reverse path
							// Else if active becomes inactive, unset path
							if(message.score){
								if(message.idnum == currPair->end1.idnum && currPair->active == 2){
									currPair->active = 1;
									printf("Reversing path\n");
									if(reversePath(currPair->path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								} else if(message.idnum == currPair->end2.idnum && currPair->active == 1){
									currPair->active = 2;
									printf("Reversing path\n");
									if(reversePath(currPair->path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								} else {
									printf("Unspecificied behavior\n");
								}
							} else { // message.score == 0
								if((message.idnum == currPair->end1.idnum && currPair->active == 1) ||
									(message.idnum == currPair->end2.idnum && currPair->active == 2)){
									printf("Unsetting path\n");
									if(unsetPath(currPair->path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									printf("Unspecified behavior\n");
								}
							}


							// if(!message.score){
							// 	printf("Node %d no longer active\n",message.idnum);
							// 	if(currPair->numIDed == 2){
							// 		printf("Unsetting path between %d and %d\n",currPair->end1.idnum,currPair->end2.idnum);
							// 		if(unsetPath(currPair->path)){
							// 			fprintf(stderr,"unsetPath() failed\n");
							// 			exit(1);
							// 		}
							// 	}
							// 	printf("Destroying pair\n"); // regardless
							// 	currPair->prev->next = currPair->next;
							// 	if(currPair == pairs){
							// 		pairs = NULL;
							// 	}
							// 	free(currPair);
							// } else {
							// 	printf("Node %d still active, no further action taken here\n",message.idnum);
							// }


						}
						if(gotUpdate){
							break;
						}
						currPair = currPair->next;
					}
					if(!gotUpdate){ // got from new node
						if(lastAssigned->numIDed == 1){
							if(within(&topdataPtr->timeBoundary,&lastAssigned->end1.time,&message.time)){
								if(message.score > lastAssigned->end1.score){
									printf("Replacing node %d\n",lastAssigned->end1.idnum);
									lastAssigned->end1 = message;
								}
							} else { // outside of time boundary
								if(message.score){
									printf("Found new end\n");
									if(assignToPair(message,pairs,&numPairs,&lastAssigned,topdataPtr)){
										fprintf(stderr,"assignToPair() failed\n");
										exit(1);
									}
								}
							}
						} else if(lastAssigned->numIDed == 2){
							if(within(&topdataPtr->timeBoundary,&lastAssigned->end2.time,&message.time)){
								if(message.score > lastAssigned->end2.score){
									printf("Replacing node %d\n",lastAssigned->end1.idnum);
									lastAssigned->end1 = message;
								}
							} else { // outside of time boundary
								if(message.score){
									printf("Found new end\n");
									if(assignToPair(message,pairs,&numPairs,&lastAssigned,topdataPtr)){
										fprintf(stderr,"assignToPair() failed\n");
										exit(1);
									}
								}
							}
						} else {
							fprintf(stderr,"shouldn't reach here\n");
							exit(1);
						}
					}
				}



				/*
				if(pair1.numIDed == 0 && pair2.numIDed == 0){ // no ends IDed
					if(message.score){
						printf("IDed pair1.end1\n");
						pair1.end1 = message;
						pair1.active = 1;
						pair1.numIDed++;
					}
				} else if(pair1.numIDed == 0 && pair2.numIDed == 1){ // pair2 partially IDed
					if(message.idnum == pair2.end1.idnum){ // got update
						if(!message.score){
							printf("pair2.end1 no longer active, de-IDing\n");
							pair2.end1.idnum = -1;
							pair2.active = -1;
							pair2.numIDed--;
						} else {
							if(message.score > pair2.end1.score){
								// printf("Reinforcing pair2.end1\n");
								// pair2.end1 = message;
							}
						}
					} else { // new node
						if(within(&topdataPtr->timeBoundary,&pair2.end1.time,&message.time)){ // topdataPtr->near[message.idnum][pair2.end1.idnum]
							if(message.score > pair2.end1.score){
								printf("Replacing pair2.end1\n");
								pair2.end1 = message;
							}
						} else { // outside of time boundary
							if(message.score && message.idnum != pair2.end1.idnum){
								printf("IDed pair2.end2\n");
								pair2.end2 = message;
								pair2.active = 2;
								pair2.numIDed++;

								printf("Setting pair2 path end2->end1\n");
								if(setpaths2(pair2.end2,pair2.end1,&pair2.path,topdataPtr->list,true)){ // true for "first path"
									fprintf(stderr,"updateConc(): setpaths2() failed\n");
									exit(1);
								}
							}
						}
					}
				} else if(pair1.numIDed == 0 && pair2.numIDed == 2){ // pair2 fully IDed (path on)
					if(message.idnum == pair2.end1.idnum || message.idnum == pair2.end2.idnum){
						if(pair2.active == 1){ // pair2 path end1->end2
							if(message.idnum == pair2.end1.idnum){
								if(!message.score){
									printf("pair2.end1 no longer active, resetting pair\n");
									pair2.end1.idnum = -1;
									pair2.end2.idnum = -1;
									pair2.active = -1;
									pair2.numIDed = 0;
									printf("Unsetting pair2 path");
									if(unsetPath(pair2.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair2.end2.idnum){
								if(message.score){
									printf("pair2.end2 now active\n");
									pair2.active = 2;
									printf("Reversing pair2 path");
									if(reversePath(pair2.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else if(pair2.active == 2){ // pair2 path end2->end1
							if(message.idnum == pair2.end2.idnum){
								if(!message.score){
									printf("pair2.end2 no longer active, resetting pair\n");
									pair2.end1.idnum = -1;
									pair2.end2.idnum = -1;
									pair2.active = -1;
									pair2.numIDed = 0;
									printf("Unsetting pair2 path");
									if(unsetPath(pair2.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair2.end1.idnum){
								if(message.score){
									printf("pair2.end1 now active\n");
									pair2.active = 1;
									printf("Reversing pair2 path");
									if(reversePath(pair2.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else {
							fprintf(stderr,"Shouldn't reach here\n");
							exit(1);
						}
					} else { // new node
						if(message.score){
							printf("IDed pair1.end1\n");
							pair1.end1 = message;
							pair1.active = 1;
							pair1.numIDed++;
						}
					}

				} else if(pair1.numIDed == 1 && pair2.numIDed == 0){ // pair1 partially IDed
					if(message.idnum == pair1.end1.idnum){ // got update
						if(!message.score){
							printf("pair1.end1 no longer active, de-IDing\n");
							pair1.end1.idnum = -1;
							pair1.active = -1;
							pair1.numIDed--;
						} else {
							if(message.score > pair1.end1.score){
								// printf("Reinforcing pair1.end1\n");
								// pair1.end1 = message;
							}
						}
					} else { // new node
						if(within(&topdataPtr->timeBoundary,&pair1.end1.time,&message.time)){ // topdataPtr->near[message.idnum][pair2.end1.idnum]
							if(message.score > pair1.end1.score){
								printf("Replacing pair1.end1\n");
								pair1.end1 = message;
							}
						} else { // outside of time boundary
							if(message.score && message.idnum != pair1.end1.idnum){
								printf("IDed pair1.end2\n");
								pair1.end2 = message;
								pair1.active = 2;
								pair1.numIDed++;

								printf("Setting pair1 path end2->end1\n");
								if(setpaths2(pair1.end2,pair1.end1,&pair1.path,topdataPtr->list,true)){ // true for "first path"
									fprintf(stderr,"updateConc(): setpaths2() failed\n");
									exit(1);
								}
							}
						}
					}
				} else if(pair1.numIDed == 1 && pair2.numIDed == 1){ // both pairs partially IDed
					fprintf(stderr,"INVALID STATE: both pairs partially IDed\n");
					exit(1);
				} else if(pair1.numIDed == 1 && pair2.numIDed == 2){ // pair1 partially IDed, pair2 fully IDed (path on)
					// merge 1-0 and 0-2 cases
					if(message.idnum == pair1.end1.idnum){ // got update
						if(!message.score){
							printf("pair1.end1 no longer active, de-IDing\n");
							pair1.end1.idnum = -1;
							pair1.active = -1;
							pair1.numIDed--;
						} else {
							if(message.score > pair1.end1.score){
								// printf("Reinforcing pair1.end1\n");
								// pair1.end1 = message;
							}
						}
					} else if(message.idnum == pair2.end1.idnum || message.idnum == pair2.end2.idnum){
						if(pair2.active == 1){ // pair2 path end1->end2
							if(message.idnum == pair2.end1.idnum){
								if(!message.score){
									printf("pair2.end1 no longer active, resetting pair\n");
									pair2.end1.idnum = -1;
									pair2.end2.idnum = -1;
									pair2.active = -1;
									pair2.numIDed = 0;
									printf("Unsetting pair2 path");
									if(unsetPath(pair2.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair2.end2.idnum){
								if(message.score){
									printf("pair2.end2 now active\n");
									pair2.active = 2;
									printf("Reversing pair2 path");
									if(reversePath(pair2.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							} else { // new node
								if(message.score){
									printf("IDed pair1.end1\n");
									pair1.end1 = message;
									pair1.active = 1;
									pair1.numIDed++;
								}
							}
						} else if(pair2.active == 2){ // pair2 path end2->end1
							if(message.idnum == pair2.end2.idnum){
								if(!message.score){
									printf("pair2.end2 no longer active, resetting pair\n");
									pair2.end1.idnum = -1;
									pair2.end2.idnum = -1;
									pair2.active = -1;
									pair2.numIDed = 0;
									printf("Unsetting pair2 path");
									if(unsetPath(pair2.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair2.end1.idnum){
								if(message.score){
									printf("pair2.end1 now active\n");
									pair2.active = 1;
									printf("Reversing pair2 path");
									if(reversePath(pair2.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else {
							fprintf(stderr,"Shouldn't reach here\n");
							exit(1);
						}
					} else { // new node
						// if(within(&topdataPtr->timeBoundary,&pair1.end1.time,&message.time)){ // topdataPtr->near[message.idnum][pair2.end1.idnum]
						// 	if(message.score > pair1.end1.score){
						// 		printf("Replacing pair1.end1\n");
						// 		pair1.end1 = message;
						// 	}
						// } else { // outside of time boundary
							if(message.score){
								printf("IDed pair1.end2\n");
								pair1.end2 = message;
								pair1.active = 2;
								pair1.numIDed++;

								printf("Setting pair1 path end2->end1\n");
								if(setpaths2(pair1.end2,pair1.end1,&pair1.path,topdataPtr->list,false)){ // true for "first path"
									fprintf(stderr,"updateConc(): setpaths2() failed\n");
									exit(1);
								}
							}
						// }
					}

				} else if(pair1.numIDed == 2 && pair2.numIDed == 0){ // pair1 fully IDed (path on)
					if(message.idnum == pair1.end1.idnum || message.idnum == pair1.end2.idnum){
						if(pair1.active == 1){ // pair1 path end1->end2
							if(message.idnum == pair1.end1.idnum){
								if(!message.score){
									printf("pair1.end1 no longer active, resetting pair\n");
									pair1.end1.idnum = -1;
									pair1.end2.idnum = -1;
									pair1.active = -1;
									pair1.numIDed = 0;
									printf("Unsetting pair1 path");
									if(unsetPath(pair1.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair1.end2.idnum){
								if(message.score){
									printf("pair1.end2 now active\n");
									pair1.active = 2;
									printf("Reversing pair1 path");
									if(reversePath(pair1.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else if(pair1.active == 2){ // pair1 path end2->end1
							if(message.idnum == pair1.end2.idnum){
								if(!message.score){
									printf("pair1.end2 no longer active, resetting pair\n");
									pair1.end1.idnum = -1;
									pair1.end2.idnum = -1;
									pair1.active = -1;
									pair1.numIDed = 0;
									printf("Unsetting pair1 path");
									if(unsetPath(pair1.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair1.end1.idnum){
								if(message.score){
									printf("pair1.end1 now active\n");
									pair1.active = 1;
									printf("Reversing pair1 path");
									if(reversePath(pair1.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else {
							fprintf(stderr,"Shouldn't reach here\n");
							exit(1);
						}
					} else { // new node
						if(message.score){
							printf("IDed pair2.end1\n");
							pair2.end1 = message;
							pair2.active = 1;
							pair2.numIDed++;
						}
					}
				} else if(pair1.numIDed == 2 && pair2.numIDed == 1){ // pair1 fully IDed (path on), pair2 partially IDed
					// merge 0-1 and 2-0 cases
					if(message.idnum == pair2.end1.idnum){ // got update
						if(!message.score){
							printf("pair2.end1 no longer active, de-IDing\n");
							pair2.end1.idnum = -1;
							pair2.active = -1;
							pair2.numIDed--;
						} else {
							if(message.score > pair2.end1.score){
								// printf("Reinforcing pair2.end1\n");
								// pair2.end1 = message;
							}
						}
					} else if(message.idnum == pair1.end1.idnum || message.idnum == pair1.end2.idnum){
						if(pair1.active == 1){ // pair1 path end1->end2
							if(message.idnum == pair1.end1.idnum){
								if(!message.score){
									printf("pair1.end1 no longer active, resetting pair\n");
									pair1.end1.idnum = -1;
									pair1.end2.idnum = -1;
									pair1.active = -1;
									pair1.numIDed = 0;
									printf("Unsetting pair1 path");
									if(unsetPath(pair1.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair1.end2.idnum){
								if(message.score){
									printf("pair1.end2 now active\n");
									pair1.active = 2;
									printf("Reversing pair1 path");
									if(reversePath(pair1.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							} else { // new node
								if(message.score){
									printf("IDed pair2.end1\n");
									pair2.end1 = message;
									pair2.active = 1;
									pair2.numIDed++;
								}
							}
						} else if(pair1.active == 2){ // pair1 path end2->end1
							if(message.idnum == pair1.end2.idnum){
								if(!message.score){
									printf("pair1.end2 no longer active, resetting pair\n");
									pair1.end1.idnum = -1;
									pair1.end2.idnum = -1;
									pair1.active = -1;
									pair1.numIDed = 0;
									printf("Unsetting pair1 path");
									if(unsetPath(pair1.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair1.end1.idnum){
								if(message.score){
									printf("pair1.end1 now active\n");
									pair1.active = 1;
									printf("Reversing pair1 path");
									if(reversePath(pair1.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else {
							fprintf(stderr,"Shouldn't reach here\n");
							exit(1);
						}
					} else { // new node
						// if(within(&topdataPtr->timeBoundary,&pair2.end1.time,&message.time)){ // topdataPtr->near[message.idnum][pair2.end1.idnum]
						// 	if(message.score > pair2.end1.score){
						// 		printf("Replacing pair2.end1\n");
						// 		pair2.end1 = message;
						// 	}
						// } else { // outside of time boundary
							if(message.score){
								printf("IDed pair2.end2\n");
								pair2.end2 = message;
								pair2.active = 2;
								pair2.numIDed++;

								printf("Setting pair2 path end2->end1\n");
								if(setpaths2(pair2.end2,pair2.end1,&pair2.path,topdataPtr->list,false)){ // true for "first path"
									fprintf(stderr,"updateConc(): setpaths2() failed\n");
									exit(1);
								}
							}
						// }
					}

				} else if(pair1.numIDed == 2 && pair2.numIDed == 2){ // both pairs fully IDed (paths on)
					// merge 2-0 and 0-2 cases
					if(message.idnum == pair1.end1.idnum || message.idnum == pair1.end2.idnum){
						if(pair1.active == 1){ // pair1 path end1->end2
							if(message.idnum == pair1.end1.idnum){
								if(!message.score){
									printf("pair1.end1 no longer active, resetting pair\n");
									pair1.end1.idnum = -1;
									pair1.end2.idnum = -1;
									pair1.active = -1;
									pair1.numIDed = 0;
									printf("Unsetting pair1 path");
									if(unsetPath(pair1.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair1.end2.idnum){
								if(message.score){
									printf("pair1.end2 now active\n");
									pair1.active = 2;
									printf("Reversing pair1 path");
									if(reversePath(pair1.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else if(pair1.active == 2){ // pair1 path end2->end1
							if(message.idnum == pair1.end2.idnum){
								if(!message.score){
									printf("pair1.end2 no longer active, resetting pair\n");
									pair1.end1.idnum = -1;
									pair1.end2.idnum = -1;
									pair1.active = -1;
									pair1.numIDed = 0;
									printf("Unsetting pair1 path");
									if(unsetPath(pair1.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair1.end1.idnum){
								if(message.score){
									printf("pair1.end1 now active\n");
									pair1.active = 1;
									printf("Reversing pair1 path");
									if(reversePath(pair1.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else {
							fprintf(stderr,"Shouldn't reach here\n");
							exit(1);
						}
					} else if(message.idnum == pair2.end1.idnum || message.idnum == pair2.end2.idnum){
						if(pair2.active == 1){ // pair2 path end1->end2
							if(message.idnum == pair2.end1.idnum){
								if(!message.score){
									printf("pair2.end1 no longer active, resetting pair\n");
									pair2.end1.idnum = -1;
									pair2.end2.idnum = -1;
									pair2.active = -1;
									pair2.numIDed = 0;
									printf("Unsetting pair2 path");
									if(unsetPath(pair2.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair2.end2.idnum){
								if(message.score){
									printf("pair2.end2 now active\n");
									pair2.active = 2;
									printf("Reversing pair2 path");
									if(reversePath(pair2.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else if(pair2.active == 2){ // pair2 path end2->end1
							if(message.idnum == pair2.end2.idnum){
								if(!message.score){
									printf("pair2.end2 no longer active, resetting pair\n");
									pair2.end1.idnum = -1;
									pair2.end2.idnum = -1;
									pair2.active = -1;
									pair2.numIDed = 0;
									printf("Unsetting pair2 path");
									if(unsetPath(pair2.path)){
										fprintf(stderr,"unsetPath() failed\n");
										exit(1);
									}
								} else {
									// ...
								}
							} else if(message.idnum == pair2.end1.idnum){
								if(message.score){
									printf("pair2.end1 now active\n");
									pair2.active = 1;
									printf("Reversing pair2 path");
									if(reversePath(pair2.path)){
										fprintf(stderr,"reversePath() failed\n");
										exit(1);
									}
								}
							}
						} else {
							fprintf(stderr,"Shouldn't reach here\n");
							exit(1);
						}
					} else { // new node
						if(message.score){
							printf("IDed pair1.end1\n");
							pair1.end1 = message;
							pair1.active = 1;
							pair1.numIDed++;
						}
					}

				} else {
					fprintf(stderr,"Shouldn't reach here\n");
					exit(1);
				}
				*/





				/* EVEN MORE GIANT STATE MACHINE

				if(end1.idnum == -1 && end2.idnum == -1 && active == -1 && end3.idnum == -1 && end4.idnum == -1){ // none identified, none active
					if(message.score){
						printf("Identified end1\n");
						end1 = message;
						active = 1;
					}
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 1 && end3.idnum == -1 && end4.idnum == -1){ // none identified, end1 active
					fprintf(stderr,"INVALID STATE: none identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 2 && end3.idnum == -1 && end4.idnum == -1){ // none identified, end2 active
					fprintf(stderr,"INVALID STATE: none identified, end2 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 3 && end3.idnum == -1 && end4.idnum == -1){ // none identified, end3 active
					fprintf(stderr,"INVALID STATE: none identified, end3 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 4 && end3.idnum == -1 && end4.idnum == -1){ // none identified, end4 active
					fprintf(stderr,"INVALID STATE: none identified, end4 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == -1 && end3.idnum == -1 && end4.idnum == -1){ // end2 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 1 && end3.idnum == -1 && end4.idnum == -1){ // end2 identified, end1 active
					fprintf(stderr,"INVALID STATE: end2 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 2 && end3.idnum == -1 && end4.idnum == -1){ // end2 identified, end2 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 3 && end3.idnum == -1 && end4.idnum == -1){ // end2 identified, end3 active
					fprintf(stderr,"INVALID STATE: end2 identified, end3 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 4 && end3.idnum == -1 && end4.idnum == -1){ // end2 identified, end4 active
					fprintf(stderr,"INVALID STATE: end2 identified, end4 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == -1 && end3.idnum == -1 && end4.idnum == -1){ // end1 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 1 && end3.idnum == -1 && end4.idnum == -1){ // end1 identified, end1 active
					if(message.idnum == end1.idnum){ // got update from end1
						if(!message.score){
							printf("End1 no longer active\n");
							active = -1;
						} else {
							if(message.score > end1.score){
								printf("Reinforcing end1\n");
								end1 = message;
							}
						}
					} else { // received from another unit
						if(within(&topdataPtr->timeBoundary,&end1.time,&message.time)){ // topdataPtr->near[message.idnum][end1.idnum]
							if(message.score > end1.score){
								printf("Replacing end1\n");
								end1 = message;
							}
						} else { // outside of time boundary
							if(message.score && message.idnum != end1.idnum){
								printf("Identified end2\n");
								end2 = message;
								active = 2;

								printf("SHOULD SET PATH END2->END1\n");
								if(setpaths2(end2,end1,topdataPtr->list,true)){ // true for "first path"
									fprintf(stderr,"updateMaster(): setpaths2() failed\n");
									exit(1);
								}
							}
						}
					}
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 2 && end3.idnum == -1 && end4.idnum == -1){ // end1 identified, end2 active
					fprintf(stderr,"INVALID STATE: end1 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 3 && end3.idnum == -1 && end4.idnum == -1){ // end1 identified, end3 active
					fprintf(stderr,"INVALID STATE: end1 identified, end3 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 4 && end3.idnum == -1 && end4.idnum == -1){ // end1 identified, end4 active
					fprintf(stderr,"INVALID STATE: end1 identified, end4 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == -1 && end3.idnum == -1 && end4.idnum == -1){ // both end1 and end2 identified, none active
					if(message.score){
						if(message.idnum == end1.idnum){
							printf("End1 active again\n");
							active = 1;
							printf("SHOULD SET PATH END1->END2, NOT YET IMPLEMENTED\n");
						} else if(message.idnum == end2.idnum){
							printf("End2 active again\n");
							active = 2;
							printf("SHOULD SET PATH END2->END1, NOT YET IMPLEMENTED\n");
						} else {
							printf("Identified end3\n");
							end3 = message;
							active = 3;
						}
					}
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 1 && end3.idnum == -1 && end4.idnum == -1){ // both end1 and end2 identified, end1 active (path end1->end2 set)
					if(message.idnum == end1.idnum){ // got update from end1
						if(!message.score){
							printf("End1 no longer active\n"); // maybe because direction flipped
							active = -1;
							printf("SHOULD UNSET PATH, NOT YET IMPLEMENTED\n");
						} else {
							if(message.score > end1.score){
								printf("Reinforcing end1\n");
								end1 = message;
							}
						}
					} else { // received from another unit
						if(within(&topdataPtr->timeBoundary,&end1.time,&message.time)){ // topdataPtr->near[message.idnum][end1.idnum]
							if(message.score > end1.score){
								printf("Replacing end1\n");
								end1 = message;
								printf("SHOULD CHANGE PATH, NOT YET IMPLEMENTED\n");
							}
						} else { // outside of time boundary
							if(message.score && message.idnum != end1.idnum && message.idnum != end2.idnum){
								printf("Identified end3\n");
								end3 = message;
								active = 3;
							}
						}
					}
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 2 && end3.idnum == -1 && end4.idnum == -1){ // both end1 and end2 identified, end2 active (path end2->end1 set)
					if(message.idnum == end2.idnum){ // got update from end2
						if(!message.score){
							printf("End2 no longer active\n"); // maybe because direction flipped
							active = -1;
							printf("SHOULD UNSET PATH, NOT YET IMPLEMENTED");
						} else {
							if(message.score > end2.score){
								printf("Reinforcing end2\n");
								end2 = message;
							}
						}
					} else { // received from another unit
						if(within(&topdataPtr->timeBoundary,&end2.time,&message.time)){ // topdataPtr->near[message.idnum][end2.idnum]
							if(message.score > end2.score){
								printf("Replacing end2\n");
								end2 = message;
								printf("SHOULD CHANGE PATH, NOT YET IMPLEMENTED\n");
							}
						} else { // outside of time boundary
							if(message.score && message.idnum != end1.idnum && message.idnum != end2.idnum){
								printf("Identified end3\n");
								end3 = message;
								active = 3;
							}
						}
					}
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 3 && end3.idnum == -1 && end4.idnum == -1){ // both end1 and end2 identified, end3 active
					fprintf(stderr,"INVALID STATE: both end1 and end2 identified, end3 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 4 && end3.idnum == -1 && end4.idnum == -1){ // both end1 and end2 identified, end4 active
					fprintf(stderr,"INVALID STATE: both end1 and end2 identified, end4 active\n");
					exit(1);

				// DOUBLED HERE

				} else if(end1.idnum == -1 && end2.idnum == -1 && active == -1 && end3.idnum != -1 && end4.idnum == -1){ // end3 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 1 && end3.idnum != -1 && end4.idnum == -1){ // end3 identified, end1 active
					fprintf(stderr,"INVALID STATE: end3 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 2 && end3.idnum != -1 && end4.idnum == -1){ // end3 identified, end2 active
					fprintf(stderr,"INVALID STATE: end3 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 3 && end3.idnum != -1 && end4.idnum == -1){ // end3 identified, end3 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 4 && end3.idnum != -1 && end4.idnum == -1){ // end3 identified, end4 active
					fprintf(stderr,"INVALID STATE: end3 identified, end4 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == -1 && end3.idnum != -1 && end4.idnum == -1){ // both end2 and end3 identified, none active
					fprintf(stderr,"INVALID STATE: both end2 and end3 identified, none active\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 1 && end3.idnum != -1 && end4.idnum == -1){ // both end2 and end3 identified, end1 active
					fprintf(stderr,"INVALID STATE: both end2 and end3 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 2 && end3.idnum != -1 && end4.idnum == -1){ // both end2 and end3 identified, end2 active
					fprintf(stderr,"INVALID STATE: both end2 and end3 identified, end2 active\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 3 && end3.idnum != -1 && end4.idnum == -1){ // both end2 and end3 identified, end3 active
					fprintf(stderr,"INVALID STATE: both end2 and end3 identified, end3 active\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 4 && end3.idnum != -1 && end4.idnum == -1){ // both end2 and end3 identified, end4 active
					fprintf(stderr,"INVALID STATE: both end2 and end3 identified, end4 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == -1 && end3.idnum != -1 && end4.idnum == -1){ // both end1 and end3 identified, none active
					fprintf(stderr,"INVALID STATE: both end1 and end3 identified, none active\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 1 && end3.idnum != -1 && end4.idnum == -1){ // both end1 and end3 identified, end1 active
					fprintf(stderr,"INVALID STATE: both end1 and end3 identified, end1 active\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 2 && end3.idnum != -1 && end4.idnum == -1){ // both end1 and end3 identified, end2 active
					fprintf(stderr,"INVALID STATE: both end1 and end3 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 3 && end3.idnum != -1 && end4.idnum == -1){ // both end1 and end3 identified, end3 active
					fprintf(stderr,"INVALID STATE: both end1 and end3 identified, end3 active\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 4 && end3.idnum != -1 && end4.idnum == -1){ // both end1 and end3 identified, end4 active
					fprintf(stderr,"INVALID STATE: both end2 and end3 identified, end4 active\n");
					exit(1);


				} else if(end1.idnum != -1 && end2.idnum != -1 && active == -1 && end3.idnum != -1 && end4.idnum == -1){ // end1/end2/end3 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 1 && end3.idnum != -1 && end4.idnum == -1){ // end1/end2/end3 identified, end1 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 2 && end3.idnum != -1 && end4.idnum == -1){ // end1/end2/end3 identified, end2 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 3 && end3.idnum != -1 && end4.idnum == -1){ // end1/end2/end3 identified, end3 active
					if(message.idnum == end3.idnum){ // got update from end3
						if(!message.score){
							// printf("End3 no longer active\n");
							// active = -1;
						} else {
							if(message.score > end3.score){
								printf("Reinforcing end3\n");
								end3 = message;
							}
						}
					} else { // received from another unit
						if(within(&topdataPtr->timeBoundary,&end3.time,&message.time)){ // topdataPtr->near[message.idnum][end3.idnum] 
							if(message.score > end3.score){
								printf("Replacing end3 with neighbor\n");
								end3 = message;
							}
						} else { // outside of neighborhood
							// if(message.score && message.idnum != end1.idnum && message.idnum != end2.idnum){
								printf("Identified end4\n");
								// end4 = message;
								end4 = topdataPtr->end4;
								active = 4;

								printf("SHOULD SET PATH\n");
								if(setpaths2(end3,end4,topdataPtr->list,false)){ // false for "not first path"
									fprintf(stderr,"updateMaster(): setpaths2() failed\n");
									exit(1);
								}
								printf("HALTING, ENTER a TO ABORT\n");
								line = NULL; size = 0;
								getline(&line,&size,stdin);
								while(line[0] != 'a'){
									free(line);
									line = NULL;
									size = 0;
									getline(&line,&size,stdin);
								}
								free(line);

								printf("Setting all to e-0\n");
								struct Line *currlinePtr = topdataPtr->list;
								char triplet[3];
								while(currlinePtr){
									setTriplet(triplet,'e','-','0');
									if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,triplet)){
										fprintf(stderr,"updateMaster(): sendQuad() failed\n");
										exit(1);
									}
									currlinePtr = currlinePtr->next;
								}

								exit(1);
							// }
						}
					}
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 4 && end3.idnum != -1 && end4.idnum == -1){ // end1/end2/end3 identified, end4 active
					fprintf(stderr,"INVALID STATE: end1/end2/end3 identified, end4 active\n");
					exit(1);

				// AGAIN DOUBLED HERE

				} else if(end1.idnum == -1 && end2.idnum == -1 && active == -1 && end3.idnum == -1 && end4.idnum != -1){ // end4 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 1 && end3.idnum == -1 && end4.idnum != -1){ // end4 identified, end1 active
					fprintf(stderr,"INVALID STATE: end4 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 2 && end3.idnum == -1 && end4.idnum != -1){ // end4 identified, end2 active
					fprintf(stderr,"INVALID STATE: end4 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 3 && end3.idnum == -1 && end4.idnum != -1){ // end4 identified, end3 active
					fprintf(stderr,"INVALID STATE: end4 identified, end3 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 4 && end3.idnum == -1 && end4.idnum != -1){ // end4 identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == -1 && end3.idnum == -1 && end4.idnum != -1){ // end2 and end4 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 1 && end3.idnum == -1 && end4.idnum != -1){ // end2 and end4 identified, end1 active
					fprintf(stderr,"INVALID STATE: both end2 and end4 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 2 && end3.idnum == -1 && end4.idnum != -1){ // end2 and end4 identified, end2 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 3 && end3.idnum == -1 && end4.idnum != -1){ // end2 and end4 identified, end3 active
					fprintf(stderr,"INVALID STATE: both end2 and end4 identified, end3 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 4 && end3.idnum == -1 && end4.idnum != -1){ // end2 and end4 identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == -1 && end3.idnum == -1 && end4.idnum != -1){ // end1 and end4 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 1 && end3.idnum == -1 && end4.idnum != -1){ // end1 and end4 identified, end1 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 2 && end3.idnum == -1 && end4.idnum != -1){ // end1 and end4 identified, end2 active
					fprintf(stderr,"INVALID STATE: both end1 and end4 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 3 && end3.idnum == -1 && end4.idnum != -1){ // end1 and end4 identified, end3 active
					fprintf(stderr,"INVALID STATE: both end1 and end4 identified, end3 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 4 && end3.idnum == -1 && end4.idnum != -1){ // end1 and end4 identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == -1 && end3.idnum == -1 && end4.idnum != -1){ // end1/end2/end4 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 1 && end3.idnum == -1 && end4.idnum != -1){ // end1/end2/end4 identified, end1 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 2 && end3.idnum == -1 && end4.idnum != -1){ // end1/end2/end4 identified, end2 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 3 && end3.idnum == -1 && end4.idnum != -1){ // end1/end2/end4 identified, end3 active
					fprintf(stderr,"INVALID STATE: end1/end2/end4 identified, end3 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 4 && end3.idnum == -1 && end4.idnum != -1){ // end1/end2/end4 identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);

				// DOUBLED HERE

				} else if(end1.idnum == -1 && end2.idnum == -1 && active == -1 && end3.idnum != -1 && end4.idnum != -1){ // end3 and end4 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 1 && end3.idnum != -1 && end4.idnum != -1){ // end3 and end4 identified, end1 active
					fprintf(stderr,"INVALID STATE: both end3 and end4 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 2 && end3.idnum != -1 && end4.idnum != -1){ // end3 and end4 identified, end2 active
					fprintf(stderr,"INVALID STATE: both end3 and end4 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 3 && end3.idnum != -1 && end4.idnum != -1){ // end3 and end4 identified, end3 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum == -1 && active == 4 && end3.idnum != -1 && end4.idnum != -1){ // end3 and end4 identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == -1 && end3.idnum != -1 && end4.idnum != -1){ // end2/end3/end4 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 1 && end3.idnum != -1 && end4.idnum != -1){ // end2/end3/end4 identified, end1 active
					fprintf(stderr,"INVALID STATE: end2/end3/end4 identified, end1 active\n");
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 2 && end3.idnum != -1 && end4.idnum != -1){ // end2/end3/end4 identified, end2 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 3 && end3.idnum != -1 && end4.idnum != -1){ // end2/end3/end4 identified, end3 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum == -1 && end2.idnum != -1 && active == 4 && end3.idnum != -1 && end4.idnum != -1){ // end2/end3/end4 identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == -1 && end3.idnum != -1 && end4.idnum != -1){ // end1/end3/end4 identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 1 && end3.idnum != -1 && end4.idnum != -1){ // end1/end3/end4 identified, end1 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 2 && end3.idnum != -1 && end4.idnum != -1){ // end1/end3/end4 identified, end2 active
					fprintf(stderr,"INVALID STATE: end1/end3/end4 identified, end2 active\n");
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 3 && end3.idnum != -1 && end4.idnum != -1){ // end1/end3/end4 identified, end3 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum == -1 && active == 4 && end3.idnum != -1 && end4.idnum != -1){ // end1/end3/end4 identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == -1 && end3.idnum != -1 && end4.idnum != -1){ // all identified, none active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 1 && end3.idnum != -1 && end4.idnum != -1){ // all identified, end1 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 2 && end3.idnum != -1 && end4.idnum != -1){ // all identified, end2 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 3 && end3.idnum != -1 && end4.idnum != -1){ // all identified, end3 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else if(end1.idnum != -1 && end2.idnum != -1 && active == 4 && end3.idnum != -1 && end4.idnum != -1){ // all identified, end4 active
					fprintf(stderr,"NOT YET DEFINED: end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				} else {
					fprintf(stderr,"INVALID STATE: else case; end1 is %d, end2 is %d, active is %d, end3 is %d, end4 is %d\n",end1.idnum,end2.idnum,active,end3.idnum,end4.idnum);
					exit(1);
				}

				*/
			}
		} // end of loop over ready sockets
	} // end of infinite loop
	return NULL; //***
}

int setAll(const char *triplet, struct Line *lst){
	printf("Setting all to %s\n",triplet);
	struct Line *currlinePtr = lst;
	// char triplet[3];
	while(currlinePtr){
		// setTriplet(triplet,'e','-','0');
		if(sendQuad(currlinePtr->host->sock,currlinePtr->idnum,(char *)triplet)){
			fprintf(stderr,"conclinks: sendQuad() failed\n");
			return 1;
		}
		currlinePtr = currlinePtr->next;
	}
	return 0;
}

/* Similar to setpaths(), except if not first path, does not clear previous path(s).
   Adjacent path setting is commented out. */
int setpaths2(struct Message2 end1, struct Message2 end2, struct Path *path, struct Line *list, bool firstPath){

	// prepare main path

	struct Paths allpaths;
	if(genPaths(&allpaths,list,end1.idnum,end2.idnum,true)){ // true for disjoint
		fprintf(stderr,"setpaths(): genPaths() failed for %d->%d\n",end1.idnum,end2.idnum);
		return 1;
	}
	if(!allpaths.count){
		fprintf(stderr,"setpaths(): no such paths exist for %d->%d\n",end1.idnum,end2.idnum);
		return 1;
	}
	// printf("Desired paths from %d to %d:\n",links.linkset[i].startid,links.linkset[i].endid);
	// printPaths(stdout,links.linkset[i].allpaths);

	struct Path minpath = allpaths.paths[0];
	for(int j = 1; j < allpaths.count; j++){
		if(allpaths.paths[j].len < minpath.len){
			minpath = allpaths.paths[j];
		}
	}
	printf("Minpath from %d->%d: ",end1.idnum,end2.idnum);
	printPath(stdout,minpath);
	*path = minpath;
	// for(int j = 0; j < minpath.len; j++){
	// 	minpath.nodes[j]->used = true;
	// }

	if(prepCmds(list,end1.antenna,minpath,'o')){ // 'o' for opposite
		fprintf(stderr,"setpaths(): prepCmds() failed\n");
		return 1;
	}

	// prepare adjacent path
	// struct Message adj1, adj2;
	// if(setAdj(&adj1,&adj2,minpath.nodes[0],minpath.nodes[minpath.len - 1])){
	// 	fprintf(stderr,"setpaths(): setAdj failed\n");
	// 	exit(1);
	// }

	freePaths(allpaths);

	// if(genPaths(&allpaths,list,adj1.idnum,adj2.idnum,true)){ // true for disjoint
	// 	fprintf(stderr,"setpaths(): genPaths() failed for %d->%d\n",adj1.idnum,adj2.idnum);
	// 	return 1;
	// }
	// if(!allpaths.count){
	// 	fprintf(stderr,"setpaths(): no such paths exist for %d->%d\n",adj1.idnum,adj2.idnum);
	// 	return 1;
	// }

	// minpath = allpaths.paths[0];
	// for(int j = 1; j < allpaths.count; j++){
	// 	if(allpaths.paths[j].len < minpath.len){
	// 		minpath = allpaths.paths[j];
	// 	}
	// }
	// printf("Minpath from %d->%d: ",adj1.idnum,adj2.idnum);
	// printPath(stdout,minpath);

	// if(prepCmds(list,adj1.antenna,minpath,adj2.antenna)){ // 'o' for opposite at 2
	// 	fprintf(stderr,"setpaths(): prepCmds() failed\n");
	// 	return 1;
	// }

	// freePaths(allpaths);

	// set paths

	if(firstPath){ // used to be !firstpath (bug?)
		struct Line *currlinePtr = list;
		while(currlinePtr){
			currlinePtr->used = false;
			currlinePtr = currlinePtr->next;
		}
	}

	/* Send commands to forwarding PCs */
	printf("Sending commands to forwading PCs...\n");
	if(firstPath){
		if(sendAllCmds2(list)){ // doesn't clear settings for next time
			fprintf(stderr,"setpaths(): sendAllCmds() failed\n");
			return 1;
		}
	} else {
		if(sendAllCmds(list)){ // clears settings after send
			fprintf(stderr,"setpaths(): sendAllCmds() failed\n");
			return 1;
		}
	}
	return 0;
}

/* Sends command triplets from main data structure for all nodes */
int sendAllCmds2(struct Line *listPtr){
	while(listPtr){
		// if(write(listPtr->host->sock,&listPtr->idnum,1) == -1){
		// 	fprintf(stderr,"sendAllCmds(): write() failed for %s\n",listPtr->host->addr);
		// 	return 1;
		// }
		printf("Sending (%d:%c%c%c) to %s\n",listPtr->idnum,listPtr->triplet[0],listPtr->triplet[1],listPtr->triplet[2],listPtr->host->addr);
		if(listPtr->triplet[0] == listPtr->triplet[1]){
			fprintf(stderr,"sendAllCmds2(): cannot set both antenna to %c\n",listPtr->triplet[0]);
			return 1;
		}
		if(sendQuad(listPtr->host->sock,listPtr->idnum,listPtr->triplet)){
			fprintf(stderr,"sendAllCmds(): sendQuad() failed for %s\n",listPtr->host->addr);
			return 1;
		}

		if(listPtr->triplet){
			listPtr->triplet[0] = 'e';
			listPtr->triplet[1] = '-';
			listPtr->triplet[2] = '0';
		}

		listPtr = listPtr->next;
	}
	return 0;
}


