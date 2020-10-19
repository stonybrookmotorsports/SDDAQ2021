#ifndef FILEWRITER_H
#define FILEWRITER_H

void csvwrite(char * fpath, int ncols, int dlen, char ** names, int ** data, char wrtnames);

void csvappend(char * fpath, int ncols, int dlen, int ** data);

void ezwrite(char * fpath, int ncols);

void zeropopulate(int ** input, int range);

void linpopulate(int ** input, int range);

#endif
