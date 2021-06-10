/* opencomms.c -- created by Will Sussman on July 24, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int *opencomms(int numgroups, int port){
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	char arduino_socket[numgroups][15];
	int suffix;
	for(int i = 0; i < numgroups; i++){
		suffix = 100 + i;
		sprintf(arduino_socket[i],"192.168.10.%d",suffix);
	}
	int *socks = malloc(sizeof(*socks) * numgroups);
	if(socks == NULL){
		fprintf(stderr,"opencomms(): malloc() failed\n");
		return NULL;
	}
	for(int i = 1; i < numgroups; i++){ //USED TO START FROM 0 FOR GENERALITY
		if((socks[i] = socket(AF_INET,SOCK_STREAM,0)) == -1){
			fprintf(stderr,"opencomms(): socket() failed\n");
			return NULL;
		}
		if(inet_pton(AF_INET,arduino_socket[i],&serv_addr.sin_addr) != 1){
			fprintf(stderr,"opencomms(): inet_pton() failed\n");
			return NULL;
		}
		if(connect(socks[i],(struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1){
			fprintf(stderr,"opencomms(): WARNING: connect() failed for socket %d (%s)\n",socks[i],arduino_socket[i]);
			// return NULL;
		} else {
			printf("opencomms(): connect() succeeded for socket %d (%s)\n",socks[i],arduino_socket[i]);
		}
	}
	return socks;
}
