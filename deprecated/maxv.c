// getv.c -- created by Will Sussman on January 21, 2020

#include <stdlib.h>
#include <unistd.h>
#include "willslib.h"

int main(int argc, char **argv){
	if(argc != 5){
		fprintf(stderr,"usage: getv ipaddr dir port timeout\n");
		fprintf(stderr,"example: getv 172.16.10.101 n 4012 0\n");
		exit(1);
	}
	char *ipaddr = argv[1];
	char dir = argv[2][0];
	int port = atoi(argv[3]);
	float timeout = atof(argv[4]);

	printf("Connecting to %s...\n",ipaddr);
	int sock = openClient(ipaddr,port,timeout);
	if(sock == -1){
		fprintf(stderr,"fwdcmds: openClient() failed for %s\n",ipaddr);
		exit(1);
	}

	float voltage, powerIndex;
	printf("Querying voltage and power index...\n");
	int stat = queryVoltage(sock,dir,&voltage,&powerIndex);
	if(!stat){
		fprintf(stderr,"getv: queryVoltage() failed to read from %s\n",ipaddr);
		exit(1);
	} else if(stat == -1){
		fprintf(stderr,"getv: queryVoltage() failed\n");
		exit(1);
	} else {
		printf("%s reported voltage of %f V, power index of %f\n",ipaddr,voltage,powerIndex);
	}

	printf("Disconnecting from %s...\n",ipaddr);
	if(close(sock)){
		fprintf(stderr,"getv: close() failed for %s\n",ipaddr);
		exit(1);
	}

	return 0;
}
