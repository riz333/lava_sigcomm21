// libwill.h -- created by Will Sussman on November 22, 2019

// 2:15p

// char *copyfield(char *row, int fieldnum);

#include <stdio.h>
#include <stdbool.h>

struct Path{
	int len;
	long *elts;
};

struct Buffer{
	int size;
	char *contents;
};

struct Entry{
	char *id;
	long idval;

	char *ipaddr;

	char *from;
	struct Path fromvals;
	// char *to;
	// struct Path tovals;

	char *dirs;
	struct Buffer dirchars;

	int socket;
};

struct List{
	struct Entry entry;
	struct List *next;
};

struct Pathlist{
	struct Path path;
	struct Pathlist *next;
};

int opencomms2(char *ipaddr, int port, int timeout, int *sockptr);

int sendcmd2(char dir1, char dir2, char phase, int sock);

int closecomms2(int sock);

int auxgenpaths(struct Path *path, struct List *list, FILE *destfile, long startid, long endid);

int append(long idval, struct Path *path);

int cellstart(int row, int col, struct Buffer buf); /*
Takes in a buffer of newline-terminated rows with tab-separated fields
Rows and columns are one-indexed
On success, returns offset from beginning of buffer
On failure, returns -1
*/

int closecomms(struct List *list);

int delete(long idval, /*struct Path *path*/struct Entry *entry);

char *dirstr(struct Buffer buf);

int fprintlist(struct List *list, FILE *file);

int freelist(struct List *list);

int freenode(struct List *node);

int genpaths2(struct Path *path, struct List *list, struct Path **pathset, long startid, long endid, int *numpaths);

int insert(/*long idval, struct Path *path*/struct Entry src, struct Entry *dest);

int listadd(struct Entry entry, struct List **list);

int listbuild(struct Entry entry, struct List **list);

bool listcheck(long idval, struct List *list);

int listdelete(long idval, struct List **list);

int listlookup(long idval, struct List *list, struct Entry *entry);

struct Buffer loadf(char *filename); /*
Takes in a filename, opens the file, determines the file size, allocates memory
for the buffer, reads the file into the buffer, and closes the file
On success, buf.contents points to the new memory (may be NULL if the file size was 0),
buf.size is set, and buf is returned
On error, buf.contents is NULL, buf.size is -1, and buf is returned
*/

int makelist(char *filename, struct List **list);

char *makesubstr(char *str, int beg, int end); /*
Takes in a string, a beginning index, and an end index
Makes a new string of size (end - beg + 1)
Copies string[beg] through string[end - 1] to new string
Sets end of new string to null character
Returns new string
On error, returns NULL
*/

bool member(long idval, struct Path path);

int memind(long idval, struct Path path);

int numdigits(long val);

int opencomms(struct List *list, int port, int timeout);

struct Buffer parsedirs(struct Buffer buf, int cellstart);

struct Path parsepath(struct Buffer buf, int cellstart); /*
Takes in a buffer and the index of the beginning of the cell to parse
Cell should contain comma-separated integers and end with tab or newline
Commas must immediately follow integers (nothing in between)
Does not check for strtol() errors
On success, path.len and path.elts set and path is returned
On failure, path.len is -1, path.elts is NULL, and path is returned
*/

char *pathstr(struct Path path);

int prepbytes(struct Path path, struct Buffer *buf, struct List *list);

int sendcmd(char startdir, char *bytes, char enddir, struct Path path, struct List *list);

int unappend(long idval, struct Path *path);
