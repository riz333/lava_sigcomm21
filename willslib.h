// willslib.h -- created by Will Sussman on December 26, 2019

#define _GNU_SOURCE
#define ARDUINO_SIZEOF_STRUCT_DATA 5
// #define ARDUINO_SIZEOF_STRUCT_MESSAGE 4
#define TABLESIZE 50

#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

/*

!!! DEPRECATED !!!

Instructions:
Run fwdcmds on the Arduino-controlling PCs
	./fwdcmds jan2top.txt 4012 0 4013 0
Then run setpath (the main algorithm) on the master controller
	./setpath jan2top.txt 4013 0 16 w 22 n

setoff works like setpath
	./setoff jan2top.txt 4013 0
setnode works only on the applicable Arduino-controlling PC
	./setnode jan2top.txt 4012 0 17 ns0

Note: Some error-checks were omitted to save time

*/

// struct Triad {
// 	float score;
// 	int idnum;
// 	char dir;
// };

struct Neighbors {
	int count; // number of neighbors
	int *idnums; // array of size Neighbors.count with ID numbers of neighbors
	struct Line **lines; // array of size Neighbors.count with pointers to neighbors
	char *dirs; // array of size Neighbors.count with directions of neighbors
};

// struct Data {
// 	// char vdir;
// 	// char pdir;
// 	float v;
// 	float p;
// };

struct Message {
	short idnum;
	short score;
	short antenna;
};

struct Message2 {
	short idnum;
	short score;
	short antenna;
	struct timeval time;
};

struct Data {
	float val;
	char dir;
};

struct Report {
	// int idnumV;
	// int idnumP;
	// struct Max vMax;
	// struct Max pMax;
	struct Data data;
	int idnum;
};

struct Host {
	char *addr; // IP address of the host
	int sock; // socket of the host
	struct Host *next; // pointer to next Host in linked list
	// struct Data data;
	// struct Report report;
};

struct Antennas {
	char *array;
	int num;
	// struct Data data;
};

struct Line {
	int idnum; // ID number of the node
	struct Host *host; // pointer to the Host that controls the node
	char *ipaddr; // IP address of the node
	struct Neighbors neighbors; // neighbors of the node
	struct Antennas nonpath;
	struct Line *next; // pointer to next node in linked list
	bool seen; // used for path generation; whether the node is already in the path
	bool used; // used for path generation; whether the node is already in a chosen path
	bool set; // used for data structure initialization; whether the block has been initialized
	bool queried; // used for best neighbor search
	int sock; // socket of the node
	char triplet[3]; // used for sending commands; holds the 3-byte command
	int clock;
	// float voltage;
	// float powerIndex;
	// struct Max maxV;
	// struct Max maxP;
	struct Antennas ants;
	float metascore;
	int history[TABLESIZE];
	int historyIndex;
	bool tableFull;
	struct sockaddr_in server;
	int listenSock;
	bool isAnEnd;
};

struct Path {
	int len; // number of nodes in the path
	struct Line **nodes; // array of size Path.len with ordered pointers to path nodes
};

struct Paths {
	int count; // number of paths
	struct Path *paths; // array of size Paths.count with a path in each cell
};

struct Topdata {
	struct Host *hosts; // head of linked list of topology Hosts
	struct Line *list; // head of linked list of nodes
	int nhosts;
	int nlocal;
	int *hostSocks;
	int *localSocks;
	char *inet_addr;
	float alertTimeout;
	int maxHost;
	int maxLocal;
	float alpha;
	struct Message msg;
	int parentSock;
	int *ardSocks;
	int **near;
	int maxID;
	bool gotCommand;
	struct timeval timeBoundary;
	// int numEnds;
	// struct Message2 end4;
	// bool stop;
};

struct Cmd {
	int idnum; // ID number to receive the command
	char triplet[3]; // the 3-byte command
};

struct Link {
	int startid;
	char startdir;
	int endid;
	char enddir;
	struct Paths allpaths;
};

struct Pair {
	int numIDed;
	struct Message2 end1;
	struct Message2 end2;
	int active;
	struct Path path;
	struct Pair *next;
	struct Pair *prev;
};

struct Links {
	int nlinks;
	struct Link *linkset;
};

struct Endpoint {
	struct Line *line;
	struct Data data;
};

/* See willslib.c for inline descriptions */
int loadTop(FILE *stream, struct Topdata *topdata); // initializes the data structure
int printTop(FILE *stream, struct Line *datalist); // dumps the data structure in topfile form
int freeTop(struct Topdata topdata); // frees the data structure
int topAdd(struct Line **topdata, struct Line *linePtr); // adds to the data structure without editing neighbor nodes
int topDelete(struct Line **topdata, struct Line *linePtr); // deletes from the data structure without editing neighbor nodes
int topInsert(struct Line **topdata, struct Line *linePtr); // adds to the data structure and edits neighbor nodes
int topRemove(struct Line **topdata, struct Line *linePtr); // deletes from the data structure and edits neighbor nodes
int parseLine(char *line, struct Topdata *topdata); // creates node from topfile line
int printLine(FILE *stream, struct Line *linePtr); // dumps node in topfile form
int freeLine(struct Line *linePtr); // frees node
int isNeighbor(int idnum, struct Neighbors neighbors, char *dirPtr); // checks if idnum is a member of neighbors, and sets *dirPtr if so
int addNeighbor(struct Line *linePtr, struct Neighbors *neighborsPtr, char dir); // adds given node to given neighbors
int deleteNeighbor(int idnum, struct Neighbors *neighborsPtr); // deletes given node from given neighbors
char oppDir(char dir); // returns the opposite cardinal direction of the input direction
struct Line *lineLookup(struct Line *datalist, int idnum); // returns a pointer to the node with ID number idnum, or NULL if not found
int genPaths(struct Paths *allpaths, struct Line *datalist, int startid, int endid, bool disjoint); // recursively generates the set of all paths from startid to endid
int genPathsAux(struct Line *currlinePtr, struct Path *path, struct Paths *pathset, int endid, bool disjoint); // genPaths() helper function
int prepCmds(struct Line *datalist, char startdir, struct Path path, char enddir); // turns path into commands, including off commands
int openClient(char *ipaddr, int port, float timeout); // returns socket for IP communication as client
int reopenServ(int listenSock, struct sockaddr_in *server); // returns new socket for IP communication as server
int openServ(int *listenSock, int port, float timeout, struct sockaddr_in *server); // returns socket for IP communication as server
int openServ2(int *listenSock, int port, float timeout, struct sockaddr_in *server, struct Topdata topdata); // returns sockets for IP communication as server
int openServ3(int *listenSock, int port, float timeout, struct sockaddr_in *server, struct Topdata topdata); // returns sockets for IP communication as server
int sendTriplet(int sock, char *triplet); // sends 3-byte command
int sendAllCmds(struct Line *listPtr); // sends commands for all nodes
int pathPush(struct Path *path, struct Line *linePtr); // add to end of path
int savePath(struct Path *path, struct Paths *pathset); // adds path to pathset
int pathPop(struct Path *path); // removes from end of path
int printPath(FILE *stream, struct Path path); // prints comma-separated path nodes
int freePaths(struct Paths pathset); // frees a set of paths
int printPaths(FILE *stream, struct Paths allpaths); // prints a set of paths
int pathcpy(struct Path *dest, struct Path *src); // duplicates source path memory
int recCmd(int recSock, struct Cmd *cmd); // gets 4-byte command
int sendQuad(int sock, int idnum, char *triplet); // sends 4-byte command
int prepOffCmds(struct Line *datalist); // makes off commands for all nodes
int parseLinks(struct Links *links, char *line);
int queryRecv(int sock, struct Data *dataBlock);
int queryUnit(int sock, char dir, struct Data *dataBlock);
int report(int sock, struct Report report);
int getReport(int sock, struct Report *report);
float average(float *vVals, int nTrials);
struct Report getMedian(struct Data *dataBlocks, int idnum, int ntrials);
void selectionSort(struct Data *dataBlocks, int ntrials);
int prepGivenCmds(struct Line *datalist, char *request);
int setQuad(char *quad, char first, char second, char third, char fourth);
int setTriplet(char *triplet, char first, char second, char third);
int broadcast(struct Host *hosts, char *quad);
int waitAlert(struct Topdata topdata, int *spikeIDptr, float alertDuration);
int waitAlert2(struct Topdata topdata, int *spikeIDptr, float alertDuration);
int bestNearby(struct Line *spikePtr, int ntrials, struct Endpoint *endptPtr);
int getUpdates(struct Endpoint endpt, float frac, char pathAnt, int redundancy);
int getUnitData(struct Line *linePtr, char dir, int ntrials, struct Data *dataPtr);
int sendQuad2(int sock, char *quad);
int readAnswer(int sock, void *ptr, size_t count);
int gotSignal(int sock);
float getMedian2(float *trialsData, int ntrials);
int compare(const void *arg1, const void *arg2);
int genericSend(int sock, void *ptr, size_t count);
int genericBroadcast(struct Host *hosts, void *ptr, size_t count);
// int queryBestVoltage(int sock, struct Max *vBlock, struct Max *pBlock);
void *updateTable(void *arg);
float recalculate(float metascore, float score, float alpha);
void *updateMaster(void *topdataPtrVoid);
void *updateConc(void *topdataPtrVoid);
char randhex(void);
int setpaths(struct Message2 end1, struct Message2 end2, struct Line *list);
int setAdj(struct Message *adj1, struct Message *adj2, struct Line *nodePtr1, struct Line *nodePtr2);
bool within(struct timeval *timeBoundaryPtr, struct timeval *firstTimePtr, struct timeval *currTimePtr);
float floatTV(struct timeval tv);
int sendAllCmds2(struct Line *listPtr);
int setpaths2(struct Message2 end1, struct Message2 end2, struct Path *path, struct Line *list, bool firstPath);
int setAll(const char *triplet, struct Line *lst);
int unsetPath(struct Path path);
int reversePath(struct Path path);
int assignToPair(struct Message2 message, struct Pair *pairs, int *numPairs, struct Pair **lastAssigned, struct Topdata *topdataPtr);
