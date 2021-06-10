/* liblawa.h -- created by Will Sussman on June 20, 2019 */

#include <stdio.h>
#include <stdbool.h>

/* Takes in a string to scan
 * Loops through chars in string until first null
 * Returns false if any scanned char is not a digit
 * Otherwise returns true */
bool alldigits(char *string);

/* */
int bin2dec(bool *bin, int len);

/* */
bool changepath(FILE *topfile, int *currpath, int currlen, int *pathIDs, int pathlen, bool **stateTable, bool *groupChanges, int numgroups, int maxpins, int *socks);

/* */
bool changestate(int idnum, bool state, FILE *topfile, bool **stateTable, bool *groupChanges, int numgroups, int maxpins);

/* */
bool closecomms(int *socks, int numgroups);

/* Takes in a 1-indexed column number and a row string
 * If extracting first column, beg = 0, loop through chars until first delimeter and set end
 * If extracting later column, count delimeters and set beg and end based on colNum
 * If a null is detected, error, return NULL
 * Otherwise create memory for extraction and return a null-terminated copy of [beg,end] */
char *extractfield(int colNum, char *row);

/* Takes in a file pointer, an int array, and the size of the int array
 * The file should have already been opene for writing
 * Prints the int array to the file with format "%d,%d,...,%d\n" (or "-\n" if none) */
bool fprintints(FILE *outfile, int *currpath, int pathlen);

/* Recursive function
 * Takes in an input file pointer, a root ID number, a field number,
 *     an output file pointer, an int array and its size, and a partial paths setting
 * currpath and pathlen should initially be NULL and 0, respectively
 * The input and output files should have already been opened for reading and writing, respectively
 * Adds the root ID number to the int array and increments the size var
 * Looks up the line for the root, extracts the field from the line, and parses the field
 * If the partial paths setting is on, the output file will include partial paths
 * Otherwise the output file will only include full paths by printing in the base case
 * The func is called recursively on each of the IDs parsed from the field extracted from the line */
bool genfan(FILE *infile, int rootID, int field, FILE *outfile, int *currpath, int pathlen, bool partialPaths);

/* */
float getgain();

/* Takes in a file pointer and an ID number
 * The file should have already been opened for reading
 * Moves to the top of the file
 * Repeatedly gets a line and checks its initial number against the ID number
 * If a match, a copy of the line is returned
 * Otherwise NULL is returned */
char *getlineid(FILE *file, int lineNum);

/* */
bool hasduplicates(int *path, int len);

/* */
int intcmp(const void *arg1ptr, const void *arg2ptr);

/* */
int *opencomms(int numgroups, int port);

/* Takes in a string with a list of numbers and pointer to an int array and its size var
 * If the string is -, int array is set to NULL, size var is set to 0, and func returns true
 * Otherwise scans string until first null char and counts commas
 * Then sets size var, creates memory for int array, fills the int array, and returns true
 * Note to self: I use a very cool trick to do the conversion! */
bool parselist(char *strlist, int **intlist, int *numInts);

/* */
int *pathcpy(int *path, int len);

/* Takes in a pointer
 * If the pointer is not NULL, it is freed and set to NULL
 * Otherwise nothing happens */
void resetptr(void *pointer);

/* Takes in current path variables and desired path variables
 * Frees the current path array, sets the current path vars to desired path vars,
 * and clears the desired path vars
 * Returns false if input var is NULL, otherwise ultimately returns true */
bool updatepathvars(int **currpath, int *currlen, int **pathIDs, int *pathlen);

// Depracated
// bool genfullpaths(FILE *infile, int basefield, int field, FILE *outfile, bool partialPaths);
// bool getpath(FILE *infile, int **intlist, int *numInts);
// bool remdup(int **list, int *size);
