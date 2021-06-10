# Makefile -- created by Will Sussman on December 10, 2019

CC		= gcc
CFLAGS	= -g3 -std=c99 -pedantic -Wall
NEW		= /home/riz3/willsLibrary/new
# OSET	= copyfield.o makesubstr.o movetocell.o

all: alg set clean # libwill.so # clean readfield.o

clean:
	rm -f *.o

# libwill.so: ${OSET}
# 	${CC} -fPIC -shared -o $@ $^

alg: alg.o
	${CC} -L${NEW} -Wl,-rpath=${NEW} -o $@ $^ -lwill

set: set.o
	${CC} -L${NEW} -Wl,-rpath=${NEW} -o $@ $^ -lwill

# copyfield.o: copyfield.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# makesubstr.o: makesubstr.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# movetocell.o: movetocell.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# # Makefile -- created by Will Sussman on June 20, 2019

# CC		= gcc
# CFLAGS	= -g3 -std=c99 -pedantic -Wall
# OSET	= alldigits.o bin2dec.o changepath.o changestate.o closecomms.o extractfield.o \
#           fprintints.o genfan.o getgain.o getlineid.o hasduplicates.o intcmp.o \
#           opencomms.o parselist.o pathcpy.o resetptr.o updatepathvars.o
#           # genfullpaths.o getpath.o remdup.o

# all: liblawa.so # newtest test willsNewTestProgram

# alldigits.o: alldigits.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# bin2dec.o: bin2dec.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# changepath.o: changepath.c liblawa.h
# 	${CC} -fPIC ${CFLAGS} -c -o $@ changepath.c

# changestate.o: changestate.c liblawa.h
# 	${CC} -fPIC ${CFLAGS} -c -o $@ changestate.c

# closecomms.o: closecomms.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# extractfield.o: extractfield.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# fprintints.o: fprintints.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# genfan.o: genfan.c liblawa.h
# 	${CC} -fPIC ${CFLAGS} -c -o $@ genfan.c

# getgain.o: getgain.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# getlineid.o: getlineid.c liblawa.h
# 	${CC} -fPIC ${CFLAGS} -c -o $@ getlineid.c

# hasduplicates.o: hasduplicates.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# intcmp.o: intcmp.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# opencomms.o: opencomms.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# parselist.o: parselist.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# pathcpy.o: pathcpy.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# # remdup.o: remdup.c liblawa.h
# # 	${CC} -fPIC ${CFLAGS} -c -o $@ remdup.c

# resetptr.o: resetptr.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# updatepathvars.o: updatepathvars.c liblawa.h
# 	${CC} -fPIC ${CFLAGS} -c -o $@ updatepathvars.c

# # genfullpaths.o: genfullpaths.c liblawa.h
# # 	${CC} -fPIC ${CFLAGS} -c -o  $@ genfullpaths.c

# # getpath.o: getpath.c liblawa.h
# # 	${CC} -fPIC ${CFLAGS} -c -o  $@ getpath.c

# liblawa.so: ${OSET}
# 	${CC} -fPIC -shared -o $@ $^

# clean:
# 	rm *.o

# # newtest.o: newtest.c liblawa.h
# # 	${CC} ${CFLAGS} -c newtest.c

# # newtest: newtest.o
# # 	${CC} -L/home/riz3/willsCode/liblawa -Wl,-rpath=/home/riz3/willsCode/liblawa -o $@ $^ -llawa

# # test.o: test.c liblawa.h
# # 	${CC} ${CFLAGS} -c test.c

# # test: test.o
# # 	${CC} -L/home/riz3/willsCode/liblawa -Wl,-rpath=/home/riz3/willsCode/liblawa -o $@ $^ -llawa

# # willsNewTestProgram.o: willsNewTestProgram.c liblawa.h
# # 	${CC} ${CFLAGS} -c willsNewTestProgram.c

# # willsNewTestProgram: willsNewTestProgram.o
# # 	${CC} -L/home/riz3/willsCode/liblawa -Wl,-rpath=/home/riz3/willsCode/liblawa -o $@ $^ -llawa -lm
