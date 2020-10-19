#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "datareader.h"
#include "filewriter.h"

void csvwrite(char * fpath, int ncols, int dlen, char ** names, int ** data, char wrtnames){
  int fd = open(fpath, O_CREAT | O_APPEND | O_RDWR, 0644);
  int i = 0;

  if(wrtnames){
    for(; i < ncols; i++){
      if(i < ncols - 1){
	write(fd, names[i], strlen(names[i]));
	write(fd, ",", 1);
      }
      else{
	write(fd, names[i], strlen(names[i]));
	write(fd, "\n", 1);
      }
    }
  }

  int r = 0;
  int c = 0;
  char * stmp = malloc(20);
  for(r = 0; r < dlen; r++){
    for(c = 0; c < ncols; c++){
      sprintf(stmp, "%d", data[c][r]);
      if(c < ncols - 1){
	write(fd, stmp, strlen(stmp));
	write(fd, ",", 1);
      }
      else{
	write(fd, stmp, strlen(stmp));
	write(fd, "\n", 1);
      }
    }
  }  

  free(stmp);
  close(fd);  
}

/*===========================
csvappend appends data onto an existing csv
===========================*/
void csvappend(char * fpath, int ncols, int dlen, int ** data){
  int fd = open(fpath, O_CREAT | O_APPEND | O_RDWR, 0644);
  int i = 0;

  int r = 0;
  int c = 0;
  char * stmp = malloc(20);
  for(r = 0; r < dlen; r++){
    for(c = 0; c < ncols; c++){
      sprintf(stmp, "%d", data[c][r]);
      if(c < ncols - 1){
	write(fd, stmp, strlen(stmp));
	write(fd, ",", 1);
      }
      else{
	write(fd, stmp, strlen(stmp));
	write(fd, "\n", 1);
      }
    }
  }  

  free(stmp);
  close(fd);  
}
  
void ezwrite(char * fpath, int ncols){
  int fd = open(fpath, O_CREAT | O_TRUNC | O_RDWR, 0644);
  write(fd, "yeet", 4);
  close(fd);  
}

void zeropopulate(int ** input, int range){
  int i = 0;
  if(range > 0){
    for(; i < range; i++){
      input[0][i] = 0;
    }
  }
}

void linpopulate(int ** input, int range){
  int i = 0;
  if(range > 0){
    for(; i < range; i++){
      input[0][i] = i;
    }
  }
}
