Control code for the LAVA wireless infrastructure
=================================================

This directory contains five subdirectories,

	deprecated/ // oldest code, not to be used
	latest/ // most recent code, should be used
	mid/ // older code, can be used
	controlCode/ // Arduino control code
	circuit_diagram/ // detailed diagrams of the LAVA multidirectional ptototype

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

Finally, the subdirectory

	circuit_diagram/

contains four circuit schematics (pertaining to the LAVA multidirectional element design)

	amplifier_selection.pdf // describes how the LAVA element activates an amplifier
	element_control_and_power_schematic.pdf // shows all connections between the RF hardware and the microcontroller unit
	element_rf_schematic.pdf // shows the RF connections
	pcb_prototype.pdf // schematic of the element's custom PCB board (RF switching and phase shifting)