Control code for the LAVA wireless infrastructure
=================================================

This directory contains four subdirectories,

	control/ // contains the LAVA control plane algorithm
	misc/ // contains miscellaneous auxiliary functions
	controlCodeMCU/ // contains the Arduino microcontroller control code
	circuit_diagram/ // contains detailed circuit diagrams of the LAVA multidirectional prototype

two code files that cannot be moved,

	willslib.c // function definitions with descriptions
	willslib.h // function prototypes

and this README.


The subdirectory

	control/

contains three code files,

	conclinks.c // implements concurrent links at master
	newalg.c // implements single links at master
	newfwd.c // bridges master to Arduinos at subcontroller

a topology file,

	finaltop.txt // describes the testbed topology

and a Makefile that generates executables.


The subdirectory

	misc/

contains three code files,

	getv.c // queries voltage at an Arduino
	setnode.c // sends commands to an Arduino
	setpath.c // single-link route selection

and a Makefile that generates executables.

The subdirectory 

       controlCodeMCU/

contains the code implemented in the arduinos to control the units

Finally, the subdirectory

	circuit_diagram/

contains four circuit schematics (pertaining to the LAVA multidirectional element design)

	amplifier_selection.pdf // describes how the LAVA element activates an amplifier
	element_control_and_power_schematic.pdf // shows all connections between the the element's hardware and the microcontroller unit
	element_rf_schematic.pdf // shows the element's RF connections
	pcb_prototype.pdf // schematic of the element's custom PCB board (RF switching and phase shifting)