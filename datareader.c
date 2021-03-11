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

  //====================SETUP FOR PORT OPENING=======================
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

  portname[8]='A';
  portname[9]='C';
  portname[10]='M';
  for(i=0;i<10;i++){
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
  
  //======================Setup for handshake=================
  char * hsbuf = malloc(2);

  //These pauses below are necessary as it allows the subcontrollers to start up
  mpause(999);
  mpause(999);
  mpause(999);

  unsigned char * speriod = malloc(3); //this stores the reading period for a SC
  speriod[0] = 0;
  speriod[1] = 0;
  speriod[2] = 0;

  int * periods = malloc(sizeof(int)*(numopen + 1)); //this stores all the periods

  unsigned char * numsen = malloc(2); //this stores the number of sensors for a SC
  numsen[0] = 0;
  numsen[1] = 0;

  int * sens = malloc(sizeof(int)*(numopen + 1)); //this stores all the number of sensors

  //======================Start  handshake=================
  //Notably, the handshake is a three-way handshake, with the characters being s, b, and m to signify the team's acronym, SBM.
  for(i=0;i<numopen;i++){
    printf("Init Handshake %d\n", i);
    write(fd[i], "s", 1);  
  
    while(1){
      printf("Handshake stalled\n");//This should only be seen once per handshake, any more signifies that the handshake is either failing or stalled.
      fflush(stdout);
      if(read(fd[i], hsbuf, 1)){
	if(hsbuf[0] == 'b'){
	  printf("Sent and received from arduino, finishing handshake\n");
	  fflush(stdout);
	  write(fd[i], "m", 1);

	  mpause(500);
	  read(fd[i], speriod, 2);
	  read(fd[i], numsen, 1);
	  periods[i] = speriod[0] + speriod[1] * 256;
	  sens[i] = numsen[0];
	  //printf("%d: period: %d sens: %d:\n", i, periods[i], sens[i]);
	  
	  break;
	}
      }
    }
  }


  //=====================Setup for Data Collection==============
  int periodmax = 0; //Variable for max period

  for (i = 0; i < numopen; i++) {
    if (periodmax < periods[i]) {
      periodmax = periods[i];
    }
  }  //This loop looks for the max period

  int * bufsizs = malloc(sizeof(int)*numopen + 1); //this stores how much should be read from each SC
  int databufsiz = 0;//this is how much data is being read total per cycle
  int packsize = 6;//this is how many bytes are in a packsize

  for (i = 0; i < numopen; i++) {
    bufsizs[i] = sens[i] * (int)(periodmax / periods[i]) * packsize;
    databufsiz = databufsiz + bufsizs[i];
  }//this determines the buffer size for each SC and adds that to the full data buffer size

  int maxbufsiz = 0;//Variable for maximum buffer size

  for (i = 0; i < numopen; i++) {
    if (maxbufsiz < bufsizs[i]) {
      maxbufsiz = bufsizs[i];
    }
  }  //This loop looks for the maximum buffer size
  
  //unsigned char * databuf = malloc(colsiz*packsize*numopen+1);
  unsigned char * databuf = malloc(databufsiz+1);
  unsigned char * dummybuf = malloc(maxbufsiz+1);//This is an intermediate buffer that exists primarily for debugging purposes. Data is written to this, and if necessary, printed/checked, then written to the data buffer.


  //These loops initialize the buffers
  for (i = 0; i < databufsiz+1; i++) {
    databuf[i] = 0;
  }

  for (i = 0; i < maxbufsiz+1; i++) {
    dummybuf[i] = 0;
  }

  int tmr = 0;

  //This array indicates how much data has been read in a given read.
  //Any value in it should not be more than the expected buffer size.
  int * readsiz = malloc(sizeof(int)*(numopen+1));
  //This array indicates how much data has been read across all reads.
  //Any value in it should not be more than the expected bufer size.
  int * readtot = malloc(sizeof(int)*(numopen+1));

  for(i=0;i<numopen+1;i++){
    readsiz[i]=0;
    readtot[i]=0;
  }//This initializes both arrays.

  //These declarations are here for determining the amount of time taken for a reading cycle.
  struct timespec tstart, tstop;
  u_int64_t timediff;

  printf("start of collection\n");
  
  while(1){
    //read once from all streams
    for(i=0;i<numopen;i++){
      clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);//This gets the time
      fflush(stdout);//Flush stdout in case data is not displayed for some reason

      //This next line tries to read as much from the given port as needed.
      //The amount actually read is returned and stored
      readsiz[i] = read(fd[i], dummybuf, bufsizs[i] - readtot[i]);      

      //This checks if any data has been read.
      //If so, it is stored in the organized buffer
      if (readsiz[i] > 0) {
	for (k = 0; k < readsiz[i]; k++) {
	  databuf[i*packsize*sens[i] + k + readtot[i]] = dummybuf[k];
	}
	readtot[i] = readtot[i] + readsiz[i];
      }

      //This checks if the amount of data read is what was expected.
      //If so, the data is reconstructed and written to the csv.
      if (bufsizs[i] - readtot[i] == 0) {
	for(k=0;k<(bufsizs[i])/packsize;k++){
	  //printf("Read from sc %d\n", i);
	  fullsen[0] = databuf[(i*sens[i]+k)*packsize] + i * 100;
	  fulltim[0] = databuf[(i*sens[i]+k)*packsize + 1] + databuf[(i*sens[i]+k)*packsize + 2] * 256 + databuf[(i*sens[i]+k)*packsize + 3] * 256 * 256;
	  fullmsg[0] = databuf[(i*sens[i]+k)*packsize + 4] + databuf[(i*sens[i]+k)*packsize + 5] * 256;

	  if(fullmsg[0] > 36000){
	    fullmsg[0] = -1 * (65535 - fullmsg[0] + 1);
	  }

	  //These store the data into the data array
	  dat[0] = fulltim;
	  dat[1] = fullsen;
	  dat[2] = fullmsg;

	  csvappend(fpath, cols, 1, dat);
	}

	readsiz[i] = 0;//Reset the amount read last run
	readtot[i] = 0;//Reset the total amount read
      }
      
    }    
    clock_gettime(CLOCK_MONOTONIC_RAW, &tstop);//This gets the time, which is used to determine read time.
    timediff = (tstop.tv_sec-tstart.tv_sec)*1000000 + (tstop.tv_nsec-tstart.tv_nsec)/1000; //This is the amount of time taken
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
