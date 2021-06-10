// opencomms.c -- created by Will Sussman on December 22, 2019

#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h> 

#include "/home/riz3/willsLibrary/new/libwill.h"

int opencomms(struct List *list, int port, int timeout){
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	// char arduino_socket[numgroups][15];
	// int suffix;
	// for(int i = 0; i < numgroups; i++){
	// 	suffix = 100 + i;
	// 	sprintf(arduino_socket[i],"192.168.10.%d",suffix);
	// }

	// int *socks = malloc(sizeof(*socks) * numgroups);
	// if(socks == NULL){
	// 	fprintf(stderr,"opencomms(): malloc() failed\n");
	// 	return NULL;
	// }

	struct List *currnode = list;
	fd_set fdset;
	struct timeval tv;
	// int so_error;
	// socklen_t len = sizeof(so_error);
	while(currnode != NULL){
		if((currnode->entry.socket = socket(AF_INET,SOCK_STREAM,0)) == -1){
			fprintf(stderr,"opencomms(): socket() failed for entry %ld\n",currnode->entry.idval);
		} else {
			if(fcntl(currnode->entry.socket,F_SETFL,O_NONBLOCK) == -1){
				fprintf(stderr,"opencomms(): fcntl() failed for entry %ld\n",currnode->entry.idval);
			} else {
				if(inet_pton(AF_INET,currnode->entry.ipaddr,&serv_addr.sin_addr) != 1){
					fprintf(stderr,"opencomms(): inet_pton() failed for entry %ld\n",currnode->entry.idval);
				} else {
					printf("Connecting to entry %ld...\n",currnode->entry.idval);
					connect(currnode->entry.socket,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
					FD_ZERO(&fdset);
					FD_SET(currnode->entry.socket,&fdset);
					tv.tv_sec = timeout;
					tv.tv_usec = 0;
					if(select(currnode->entry.socket + 1, NULL, &fdset, NULL, &tv) == 0){ //timeout
						fprintf(stderr,"opencomms(): connect() timed out for entry %ld\n",currnode->entry.idval);
						// 1:
						// getsockopt(currnode->entry.socket,SOL_SOCKET,SO_ERROR,&so_error,&len);
						// if(so_error == 0){
						// 	printf("%s:%d is open\n",currnode->entry.ipaddr,port);
						// }
					}
					// if(connect(currnode->entry.socket,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1){
					// 	fprintf(stderr,"opencomms(): connect() failed for entry %ld\n",currnode->entry.idval);
					// }
				}
			}
		}
		currnode = currnode->next;
	}

	return 0;

	// for(int i = 1; i < numgroups; i++){ //USED TO START FROM 0 FOR GENERALITY
	// 	if((socks[i] = socket(AF_INET,SOCK_STREAM,0)) == -1){
	// 		fprintf(stderr,"opencomms(): socket() failed\n");
	// 		return NULL;
	// 	}
	// 	if(inet_pton(AF_INET,arduino_socket[i],&serv_addr.sin_addr) != 1){
	// 		fprintf(stderr,"opencomms(): inet_pton() failed\n");
	// 		return NULL;
	// 	}
	// 	if(connect(socks[i],(struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1){
	// 		fprintf(stderr,"opencomms(): WARNING: connect() failed for socket %d (%s)\n",socks[i],arduino_socket[i]);
	// 		// return NULL;
	// 	} else {
	// 		printf("opencomms(): connect() succeeded for socket %d (%s)\n",socks[i],arduino_socket[i]);
	// 	}
	// }
	// return socks;
}
