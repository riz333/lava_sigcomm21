// sendcmd2.c -- created by Will Sussman on December 30, 2019

#include <unistd.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

//	sendcmd2(dir1,dir2,phase,sock);
// int sendcmd(char startdir, char *bytes, char enddir, struct Path path, struct List *list){
int sendcmd2(char dir1, char dir2, char phase, int sock){
	// struct Entry entry;
	char triplet[3];
	// for(int i = 0; i < path.len; i++){
		// listlookup(path.elts[i],list,&entry);
		// fprintf(stdout,"%s: ",entry.ipaddr);
		// if(i == 0){ //start
		// 	triplet[0] = startdir;
		// 	triplet[1] = bytes[0];
		// 	triplet[2] = '0';
		// } else if(i == path.len - 1){ //end
		// 	triplet[0] = bytes[2 * path.len - 3];
		// 	triplet[1] = enddir;
		// 	triplet[2] = '0';
		// } else {
		// 	triplet[0] = bytes[2*i - 1];
		// 	triplet[1] = bytes[2*i];
		// 	triplet[2] = '0';
		// }
		triplet[0] = dir1;
		triplet[1] = dir2;
		triplet[2] = phase;
		printf("Sending \'%c%c%c\'",triplet[0],triplet[1],triplet[2]);
		if(write(sock,triplet,3) == -1){
			fprintf(stderr,"sendcmd2(): write failed\n");
		}
		// write(STDOUT_FILENO,triplet,3);
		// fprintf(stdout,"\n");
	// }
	return 0;
}