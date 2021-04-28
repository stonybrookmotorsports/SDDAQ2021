#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "datareader.h"
#include "filewriter.h"
#include "aux.h"

/*=============================================================
csvwrite() creates a csv file (or appends to it), and writes various data arrays to it. This function is specifically used in the initial creation of the csv file and writes the column names, as well as 1's for each of the first values.
=============================================================*/

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
  fsync(fd);
  close(fd);  
}

/*===========================
csvappend() appends data onto an existing csv. 
csvappend() only writes data, and does not work to put in the column headers
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
  fsync(fd);
  close(fd);  
}


/*==================================================
These following functions are deprecated. They were primarily used to test writing data to CSV files. 
ezwrite() was used to test writing. 
zeropopulate() would populate an integer array with zeros, 
linpopulate() would populate an integer array with the value i, where i is the index of said element.
Of note, these functions are primarily carry-over from the first attempt that Christopher "Red" Sherling made at sending data from an arduino to a computer/RPi and storing it as a CSV. These initial attempts occured during the 2018-2019 school year.
  =================================================*/
void ezwrite(char * fpath, int ncols){
  int fd = open(fpath, O_CREAT | O_TRUNC | O_RDWR, 0644);
  write(fd, "test", 4);
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
