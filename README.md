Control code for the LAVA wireless infrastructure
=================================================

This directory contains four subdirectories,

	deprecated/ // oldest code, not to be used
	latest/ // most recent code, should be used
	mid/ // older code, can be used
	controlCode/ // Arduino control code

two code files that cannot be moved,

	willslib.c // function definitions with descriptions
	willslib.h // function prototypes

and this README.


The subdirectory

	latest/

contains three code files,

	conclinks.c // implements concurrent links at master
	newalg.c // implements single links at master
	newfwd.c // bridges master to Arduinos at subcontroller

a topology file,

	finaltop.txt // describes the testbed topology

and a Makefile that generates executables.


The subdirectory

	mid/

contains five code files,

	algorithm.c // older single-link implementation
	fwdcmds.c // older Arduino-subcontroller bridge
	getv.c // queries voltage at an Arduino
	setnode.c // sends command triplet to an Arduino
	setpath.c // older single-link implementation

and a Makefile that generates executables.

The subdirectory 

       controlCode/

contains the code implemented in the arduinos to control the units
