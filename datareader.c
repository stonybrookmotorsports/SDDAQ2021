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

/*=============================
pause makes the system wait for some value millis=(0,1000) milliseconds
=============================*/
void mpause(int millis){
  struct timespec ts, ts2;
  ts.tv_sec=0;
  ts.tv_nsec = millis * 1000000L;
  nanosleep(&ts,&ts2);
  //printf("%d\n", nanosleep(&ts,&ts2));
  //printf("%d\n", errno);
}

/*=============================
configtty sets up the connection to the USB ports. A file descriptor fd of the port file, and the baudrate br (passed as B######, so B115200 for example) sets the read rate. 
=============================*/
int configtty(int fd, int br){
  struct termios tty;

  tcgetattr(fd,&tty);
  
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;
  
  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ECHOE;
  tty.c_lflag &= ~ECHONL;
  tty.c_lflag &= ~ISIG;

  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF|IXANY);

  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;

  tty.c_cc[VMIN] = 1;
  tty.c_cc[VMIN] = 1;

  cfsetospeed(&tty, (speed_t)br);
  cfsetispeed(&tty, (speed_t)br);

  tcsetattr(fd, TCSANOW, &tty);  
}

/*=============================
=============================*/
int main(){
  //=====================Create Output File=================
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


  //=========================Reconstruct data=====================
  int k;
  int * fulltim = malloc(colsiz * sizeof(int));
  int * fullmsg = malloc(colsiz * sizeof(int));

  int * init = malloc(sizeof(int));
  init[0] = 1;

  
  //=====================Real CSV Hours out here===================
  int cols = 2;
  char ** names = malloc(colsiz*cols);//size arbitrary
  int ** dat = malloc(2*sizeof(int));
  printf("Writing to CSV \n");
  fflush(stdout);
    
  names[0] = "time";
  names[1] = "rpm";

  dat[0] = init;
  dat[1] = init;
  
  csvwrite(fpath, cols, 2, names, dat, nmwrt);

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
  char opensuccess = 0;
  char numopen = 0;
  for(;i<10;i++){
    portname[11]=c+i;
    fdtests[i] = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if(fdtests[i]>0){
      fd[numopen]=fdtests[i];
      numopen = numopen + 1;
      opensuccess = 1;
    }
  }
  
  if(opensuccess=0){
    printf("Error in scanning for ports\n");
  }

  printf("Number of connected subcontrollers:%d\n", numopen);
  free(portname);
  free(fdtests);
  /*baudrate 115200, 8 bits, no parity, 1 stop bit */
  configtty(fd[0], B115200);

  //======================Start Handshake=================
  char * hsbuf = malloc(2);
  mpause(999);
  mpause(999);
  mpause(999);

  i = 0;
  for(;i<numopen;i++){
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
  unsigned char * timfh = malloc(colsiz + 1);
  unsigned char * timsh = malloc(colsiz + 1);
  unsigned char * msgfh = malloc(colsiz + 1);
  unsigned char * msgsh = malloc(colsiz + 1);

  int tmr = 0;
  int rsiz = 0;
  int rtot = 0;

  struct timespec tstart, tstop;
  u_int64_t timediff;
  
  while(1){
    clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);
    //for(tmr = 0; tmr < colsiz; tmr++){
      //printf("loop\n");

    //  mpause(250);
    //}
    printf("Collecting\n");
    fflush(stdout);
    
    rsiz = 0;
    rtot = 0;
    
    timfh[colsiz + 1] = 0;
    timsh[colsiz + 1] = 0;
    msgfh[colsiz + 1] = 0;
    msgsh[colsiz + 1] = 0;

    rsiz = read(fd[0], timfh, colsiz);
    rtot = rtot + rsiz;
    while(rtot < colsiz){
      //printf("rtot1:%d\n", rtot);
      rtot = rtot + read(fd[0], &(timfh[rtot]), colsiz-rtot);
    }
    /* printf("timfh:"); */
    /* for(k=0;k<colsiz;k++){ */
    /*   printf(" %d", timfh[k]);  */
    /* } */
    /* printf("\n"); */

    rtot = 0;
    rsiz = read(fd[0], timsh, colsiz);
    rtot = rtot+rsiz;
    while(colsiz-rtot){
      //printf("rtot2:%d\n", rtot);
      rtot = rtot + read(fd[0], &(timsh[rtot]), colsiz-rtot);
    }
    /* printf("timsh:"); */
    /* for(k=0;k<colsiz;k++){ */
    /*   printf(" %d", timsh[k]);  */
    /* } */
    /* printf("\n"); */

    rtot = 0;
    rsiz = read(fd[0], msgfh, colsiz);
    rtot = rtot+rsiz;
    while(colsiz-rtot){
      //printf("rtot3:%d\n", rtot);
      rtot = rtot + read(fd[0], &(msgfh[rtot]), colsiz-rtot);
    }
    /* printf("msgfh:"); */
    /* for(k=0;k<colsiz;k++){ */
    /*   printf(" %d", msgfh[k]);  */
    /* } */
    /* printf("\n"); */

    rtot = 0;
    rsiz = read(fd[0], msgsh, colsiz);
    rtot = rtot+rsiz;
    while(colsiz-rtot){
      //printf("rtot4:%d\n", rtot);
      rtot = rtot + read(fd[0], &(msgsh[rtot]), colsiz-rtot);
    }
    /* printf("msgsh:"); */
    /* for(k=0;k<colsiz;k++){ */
    /*   printf(" %d", msgsh[k]);  */
    /* } */
    /* printf("\n"); */

    //read(fd[0], timfh, colsiz);
    //read(fd[0], timsh, colsiz);
    //read(fd[0], msgfh, colsiz);
    //read(fd[0], msgsh, colsiz);
    
    for(k = 0; k < colsiz; k++){
      fulltim[k] = timfh[k] + timsh[k] *256;
      fullmsg[k] = msgfh[k] + msgsh[k] *256;
    }

    dat[0] = fulltim;
    dat[1] = fullmsg;
    
    csvappend(fpath, cols, colsiz, dat);

    clock_gettime(CLOCK_MONOTONIC_RAW, &tstop);
    timediff = (tstop.tv_sec-tstart.tv_sec)*1000000 + (tstop.tv_nsec-tstart.tv_nsec)/1000;
    printf("Time Taken: %lu\n", timediff);
  }
  
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
