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

  strcat(portname, "/dev/ttyACM");
  portname[12] = 0;

  //=====SCAN FOR ALL OPEN PORTS, ASSIGN FILE DESCRIPTORS====
  char c=48;
  int i=0;
  char numopen = 0;
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
      printf("Querying\n");
      fflush(stdout);
      if(read(fd[i], hsbuf, 1)){
	if(hsbuf[i] == 'b'){
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
  unsigned char * senid = malloc(colsiz + 1);
  unsigned char * timfh = malloc(colsiz + 1);
  unsigned char * timsh = malloc(colsiz + 1);
  unsigned char * timth = malloc(colsiz + 1);
  unsigned char * msgfh = malloc(colsiz + 1);
  unsigned char * msgsh = malloc(colsiz + 1);

  senid[colsiz + 1] = 0;
  timfh[colsiz + 1] = 0;
  timsh[colsiz + 1] = 0;
  timth[colsiz + 1] = 0;
  msgfh[colsiz + 1] = 0;
  msgsh[colsiz + 1] = 0;
  
  int tmr = 0;
  int rsiz = 0;
  int rtot = 0;

  struct timespec tstart, tstop;
  u_int64_t timediff;
  
  while(1){
    clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);

    printf("Collecting\n");
    fflush(stdout);
    
    rsiz = 0;
    rtot = 0;

    //===================BEGIN READING=====================
    
    rsiz = read(fd[0], senid, colsiz);
    rtot = rtot + rsiz;
    while(rtot < colsiz){
      rtot = rtot + read(fd[0], &(senid[rtot]), colsiz-rtot);
    }

    rsiz = read(fd[0], timfh, colsiz);
    rtot = rtot + rsiz;
    while(rtot < colsiz){
      rtot = rtot + read(fd[0], &(timfh[rtot]), colsiz-rtot);
    }

    rtot = 0;
    rsiz = read(fd[0], timsh, colsiz);
    rtot = rtot+rsiz;
    while(colsiz-rtot){
      rtot = rtot + read(fd[0], &(timsh[rtot]), colsiz-rtot);
    }

    rtot = 0;
    rsiz = read(fd[0], timth, colsiz);
    rtot = rtot+rsiz;
    while(colsiz-rtot){
      rtot = rtot + read(fd[0], &(timth[rtot]), colsiz-rtot);
    }

    rtot = 0;
    rsiz = read(fd[0], msgfh, colsiz);
    rtot = rtot+rsiz;
    while(colsiz-rtot){
      rtot = rtot + read(fd[0], &(msgfh[rtot]), colsiz-rtot);
    }

    rtot = 0;
    rsiz = read(fd[0], msgsh, colsiz);
    rtot = rtot+rsiz;
    while(colsiz-rtot){
      rtot = rtot + read(fd[0], &(msgsh[rtot]), colsiz-rtot);
    }

    //===================END READING=====================    

    //===================BEGIN DATA RECONSTRUCTION===============
    
    for(k = 0; k < colsiz; k++){
      fulltim[k] = timfh[k] + timsh[k] *256 + timth[k] * 256 * 256;
      fullsen[k] = senid[k];
      fullmsg[k] = msgfh[k] + msgsh[k] *256;
    }

    dat[0] = fulltim;
    dat[1] = fullsen;
    dat[2] = fullmsg;
    
    csvappend(fpath, cols, colsiz, dat);

    clock_gettime(CLOCK_MONOTONIC_RAW, &tstop);
    timediff = (tstop.tv_sec-tstart.tv_sec)*1000000 + (tstop.tv_nsec-tstart.tv_nsec)/1000;
    printf("Time Taken: %lu\n", timediff);
  }

  //===================Free Allocated Memory===============
  
  free(timfh);
  free(timsh);
  free(msgfh);
  free(msgsh);
    
  free(names);
  free(dat);
  free(fulltim);
  free(fullmsg);
  free(fd);
}
