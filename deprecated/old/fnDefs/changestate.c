/* changestate.c -- created by Will Sussman on July 22, 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "liblawa.h"

/* */
bool changestate(int idnum, bool state, FILE *topfile, bool **stateTable, bool *groupChanges, int numgroups, int maxpins){
	if(stateTable == NULL){
		fprintf(stderr,"changestate(): statefile is NULL\n");
		return false;
	}
	if(groupChanges == NULL){
		fprintf(stderr,"changestate(): rowChanges is NULL\n");
		return false;
	}

	char *line = getlineid(topfile,idnum);
	char *groupstr = extractfield(2,line);
	char *pinstr = extractfield(3,line);
	resetptr(line);
	int group = atoi(groupstr);
	if(group < 0 || group >= numgroups){
		fprintf(stderr,"changestate(): invalid group number\n");
		return false;
	}
	resetptr(groupstr);
	int pin = atoi(pinstr);
	if(pin < 0 || pin >= maxpins){
		fprintf(stderr,"changestate(): invalid pin number\n");
		return false;
	}
	resetptr(pinstr);

	stateTable[group-1][pin] = state;
	groupChanges[group-1] = 1;
	return true;
}