#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "datareader.h"
#include "filewriter.h"
#include "aux.h"

/*=============================
  =============================*/
int main(){
  //=====================Create Output File Name===================
  time_t current_time = time(NULL);

  char * fpath = ctime(&current_time);
  fpath[24] = '.';
  fpath[25] = 'c';
  fpath[26] = 's';
  fpath[27] = 'v';
  fpath[28] = 0;
  
  printf("%s\n\n\n", fpath);
  fflush(stdout);  
  
  char nmwrt = 1;
  int colsiz = 1;


  //=========================Generate filler for csv write=====================
  int k;
  int * fulltim = malloc(colsiz * sizeof(int));
  int * fullsen = malloc(colsiz * sizeof(int));
  int * fullmsg = malloc(colsiz * sizeof(int));

  int * init = malloc(sizeof(int));
  init[0] = 1;

  
  //=====================Create the CSV file===================
  int cols = 3;
  char ** names = malloc(colsiz*cols);//size arbitrary
  int ** dat = malloc(cols*sizeof(int));
    
  names[0] = "time";
  names[1] = "senid";
  names[2] = "rpm";

  dat[0] = init;
  dat[1] = init;
  dat[2] = init;
  
  csvwrite(fpath, cols, 1, names, dat, nmwrt);

  printf("Welcome to SBMDAQ 1.0.0, please enter a value between 0-9 of the COM (windows) or TTYACM (Ubuntu/MAC) port that you will be utilizing:\n");

  //====================Opening Port=======================
  char *portname = malloc(15);
  int iter = 0;
  for(;iter < 15; iter++){
    portname[0] = 0;
  }
  int * fdtests = malloc(10*sizeof(int));
  int * fd = malloc(10*sizeof(int));
  int wlen;
  int exec = 0;
  pid_t pid;
  int status;

  strcat(portname, "/dev/ttyUSB");
  portname[12] = 0;

  //=====SCAN FOR ALL OPEN PORTS, ASSIGN FILE DESCRIPTORS====
  char c=48;
  int i=0;
  int j=0;
  int numopen = 0;
  for(;i<10;i++){
    portname[11]=c+i;
    fdtests[i] = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if(fdtests[i]>0){
      fd[numopen]=fdtests[i];
      numopen = numopen + 1;
    }
  }
  
  if(!numopen){
    printf("Error in scanning for ports, exiting program\n");
    return(0);
  }

  printf("Number of connected subcontrollers:%d\n", numopen);
  free(portname);
  free(fdtests);

  //================Setup Serial Comms With Subcontrollers=======--==
  for(i=0;i<numopen;i++){
    configtty(fd[i], B115200);
  }
  
  //======================Start Handshake=================
  char * hsbuf = malloc(2);
  mpause(999);
  mpause(999);
  mpause(999);

  for(i=0;i<numopen;i++){
    printf("Init Handshake %d\n", i);
    write(fd[i], "s", 1);  
  
    while(1){
      printf("stucc\n");
      fflush(stdout);
      if(read(fd[i], hsbuf, 1)){
	if(hsbuf[0] == 'b'){
	  printf("Sent and received from arduino, finishing handshake\n");
	  fflush(stdout);
	  write(fd[i], "m", 1);
	  break;
	}
      }
    }
  }


  //=====================Data Collected=============
  //strt
  int packsize = 6;
  
  unsigned char * databuf = malloc(colsiz*packsize*numopen+1);
  
  for(i=0;i<numopen*(colsiz*packsize+1);i++){
    databuf[i]=0;
  }

  int tmr = 0;

  int * readsiz = malloc(sizeof(int)*(numopen+1));
  int * readtot = malloc(sizeof(int)*(numopen+1));

  for(i=0;i<numopen+1;i++){
    readsiz[i]=0;
    readtot[i]=0;
  }
  
  struct timespec tstart, tstop;
  u_int64_t timediff;

  while(1){
    //read once from all streams
    for(i=0;i<numopen;i++){
      clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);
      fflush(stdout);

      readsiz[i] = read(fd[i], &(databuf[i*packsize*colsiz+readtot[i]]), packsize*colsiz-readtot[i]);

      //if anything was read, increment the total number of bytes read      
      if(readsiz[i]>0){
	readtot[i] = readtot[i] + readsiz[i];
      }

      //reconstruct and store the data
      if(packsize*colsiz-readtot[i] == 0){
	fullsen[0] = databuf[i*colsiz*packsize]+i;
	fulltim[0] = databuf[i*colsiz*packsize+1] + databuf[i*colsiz*packsize+2] *256 + databuf[i*colsiz*packsize+3] * 256 * 256;
	fullmsg[0] = databuf[i*colsiz*packsize+4] + databuf[i*colsiz*packsize+5] *256;
	dat[0] = fulltim;
	dat[1] = fullsen;
	dat[2] = fullmsg;
    
	csvappend(fpath, cols, colsiz, dat);

	readsiz[i]=0;
	readtot[i]=0;	
      }  
    }    
    clock_gettime(CLOCK_MONOTONIC_RAW, &tstop);
    timediff = (tstop.tv_sec-tstart.tv_sec)*1000000 + (tstop.tv_nsec-tstart.tv_nsec)/1000;
    //printf("Time Taken: %lu\n", timediff);

  }

  //===================Free Allocated Memory===============
  
  free(names);
  free(dat);
  free(databuf);
  free(readtot);
  free(readsiz);
  free(fullsen);
  free(fulltim);
  free(fullmsg);
  free(fd);
}
