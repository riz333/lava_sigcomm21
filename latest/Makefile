# Makefile -- created by Will Sussman on December 27, 2019

CC		= gcc
CFLAGS	= -g3 -std=c99 -pedantic -Wall

all: conclinks newalg newfwd clean

clean:
	rm -f *.o *.a /home/riz3/willsLibrary/willslib.o

/home/riz3/willsLibrary/willslib.o: /home/riz3/willsLibrary/willslib.c /home/riz3/willsLibrary/willslib.h

fwdcmds: fwdcmds.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -o $@ $^
fwdcmds.o: /home/riz3/willsLibrary/willslib.h

fwdcmds2: fwdcmds2.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -o $@ $^
fwdcmds2.o: /home/riz3/willsLibrary/willslib.h

setnode: setnode.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -o $@ $^
setnode.o: /home/riz3/willsLibrary/willslib.h

# setoff: setoff.o /home/riz3/willsLibrary/willslib.o
# 	${CC} ${CFLAGS} -o $@ $^
# setoff.o: /home/riz3/willsLibrary/willslib.h

setpath: setpath.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -o $@ $^
setpath.o: /home/riz3/willsLibrary/willslib.h

algorithm: algorithm.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -o $@ $^
algorithm.o: /home/riz3/willsLibrary/willslib.h

getv: getv.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -o $@ $^
getv.o: /home/riz3/willsLibrary/willslib.h

newfwd: newfwd.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -pthread -o $@ $^
newfwd.o: /home/riz3/willsLibrary/willslib.h

newalg: newalg.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -pthread -o $@ $^
newalg.o: /home/riz3/willsLibrary/willslib.h

conclinks: conclinks.o /home/riz3/willsLibrary/willslib.o
	${CC} ${CFLAGS} -pthread -o $@ $^
conclinks.o: /home/riz3/willsLibrary/willslib.h
