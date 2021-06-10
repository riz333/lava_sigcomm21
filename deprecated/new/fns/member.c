// member.c -- created by Will Sussman on December 20, 2019

#include <stdbool.h>
#include "/home/riz3/willsLibrary/new/libwill.h"

bool member(long idval, struct Path path){
	for(int i = 0; i < path.len; i++){
		if(idval == path.elts[i]){
			return true;
		}
	}
	return false;
}