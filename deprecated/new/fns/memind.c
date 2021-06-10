// memind.c -- created by Will Sussman on December 21, 2019

#include "/home/riz3/willsLibrary/new/libwill.h"

int memind(long idval, struct Path path){
	for(int i = 0; i < path.len; i++){
		if(idval == path.elts[i]){
			return i;
		}
	}
	return -1;
}