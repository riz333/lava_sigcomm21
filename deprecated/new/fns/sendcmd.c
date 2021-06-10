// sendcmd.c -- created by Will Sussman on December 21, 2019

#include <unistd.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

int sendcmd(char startdir, char *bytes, char enddir, struct Path path, struct List *list){
	struct Entry entry;
	char triplet[3];
	for(int i = 0; i < path.len; i++){
		listlookup(path.elts[i],list,&entry);
		// fprintf(stdout,"%s: ",entry.ipaddr);
		if(i == 0){ //start
			triplet[0] = startdir;
			triplet[1] = bytes[0];
			triplet[2] = '0';
		} else if(i == path.len - 1){ //end
			triplet[0] = bytes[2 * path.len - 3];
			triplet[1] = enddir;
			triplet[2] = '0';
		} else {
			triplet[0] = bytes[2*i - 1];
			triplet[1] = bytes[2*i];
			triplet[2] = '0';
		}
		printf("Sending \'%c%c%c\' to entry %ld\n",triplet[0],triplet[1],triplet[2],entry.idval);
		if(write(entry.socket,triplet,3) == -1){
			fprintf(stderr,"sendcmd(): write failed for entry %ld\n",entry.idval);
		}
		// write(STDOUT_FILENO,triplet,3);
		// fprintf(stdout,"\n");
	}
	return 0;
}