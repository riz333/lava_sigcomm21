# Makefile -- created by Will Sussman on November 22, 2019

CC		= gcc
CFLAGS	= -g3 -std=c99 -pedantic -Wall
OSET	= auxgenpaths.o append.o cellstart.o closecomms.o delete.o dirstr.o freelist.o freenode.o fprintlist.o genpaths2.o \
          insert.o listbuild.o listcheck.o listdelete.o listlookup.o loadf.o makelist.o listadd.o makesubstr.o \
          member.o memind.o numdigits.o opencomms.o parsedirs.o parsepath.o pathstr.o prepbytes.o sendcmd.o unappend.o \
          opencomms2.o sendcmd2.o closecomms2.o # copyfield.o
NEW		= /home/riz3/willsLibrary/new
FNS		= ${NEW}/fns

all: ${NEW}/libwill.so clean # test # clean readfield.o

${NEW}/libwill.so: ${OSET}
	${CC} -fPIC -shared -o $@ $^

clean:
	rm -f *.o

# test: test.o
# 	${CC} -L/home/riz3/willsLibrary/new -Wl,-rpath=/home/riz3/willsLibrary/new -o $@ $^ -lwill

# copyfield.o: copyfield.c
# 	${CC} -fPIC ${CFLAGS} -c -o $@ $^

# int opencomms2(char *ipaddr, int port, int timeout);

# int sendcmd2(char dir1, char dir2, char phase, int sock);

# int closecomms2(int sock);

opencomms2.o: ${FNS}/opencomms2.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

sendcmd2.o: ${FNS}/sendcmd2.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

closecomms2.o: ${FNS}/closecomms2.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

auxgenpaths.o: ${FNS}/auxgenpaths.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

append.o: ${FNS}/append.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

cellstart.o: ${FNS}/cellstart.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

closecomms.o: ${FNS}/closecomms.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

dirstr.o: ${FNS}/dirstr.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

delete.o: ${FNS}/delete.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

freelist.o: ${FNS}/freelist.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

freenode.o: ${FNS}/freenode.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

fprintlist.o: ${FNS}/fprintlist.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

genpaths2.o: ${FNS}/genpaths2.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

insert.o: ${FNS}/insert.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

listadd.o: ${FNS}/listadd.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

listbuild.o: ${FNS}/listbuild.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

listcheck.o: ${FNS}/listcheck.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

listdelete.o: ${FNS}/listdelete.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

listlookup.o: ${FNS}/listlookup.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

loadf.o: ${FNS}/loadf.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

makelist.o: ${FNS}/makelist.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

makesubstr.o: ${FNS}/makesubstr.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

member.o: ${FNS}/member.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

memind.o: ${FNS}/memind.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

numdigits.o: ${FNS}/numdigits.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

opencomms.o: ${FNS}/opencomms.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

parsedirs.o: ${FNS}/parsedirs.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

parsepath.o: ${FNS}/parsepath.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

pathstr.o: ${FNS}/pathstr.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

prepbytes.o: ${FNS}/prepbytes.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

sendcmd.o: ${FNS}/sendcmd.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

unappend.o: ${FNS}/unappend.c
	${CC} -fPIC ${CFLAGS} -c -o $@ $^

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
