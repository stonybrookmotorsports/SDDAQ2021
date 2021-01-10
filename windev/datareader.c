#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys\timeb.h>
#include "datareader.h"
#include "filewriter.h"
#include "auxf.h"

/*=============================
  =============================*/
int main() {
  //=====================Create Output File Name===================
  time_t current_time = time(NULL);

  char * fpath = malloc(100);
  fpath[0] = 0;
  sprintf(fpath, "%ld", (int)current_time);
  fflush(stdout);
  strcat(fpath, ".csv");

  /*
char * fpath = ctime(&current_time);
fpath[24] = '.';
fpath[25] = 'c';
fpath[26] = 's';
fpath[27] = 'v';
fpath[28] = 0;
  */

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
  int ** dat = malloc(cols * sizeof(int));

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
  for (; iter < 15; iter++) {
    portname[0] = 0;
  }
  int * fdtests = malloc(10 * sizeof(int));
  int * fd = malloc(10 * sizeof(int));
  int wlen;
  int exec = 0;
  int status;

  strcat(portname, "COM");
  portname[4] = 0;
  portname[12] = 0;

  //=====SCAN FOR ALL OPEN PORTS, ASSIGN FILE DESCRIPTORS====
  char c = 48;
  int i = 0;
  int j = 0;
  int numopen = 0;

  //int flags = 0;

  for (; i < 10; i++) {
    portname[3] = c + i;
    printf("Port: %s C: %c %d i: %d\n", portname, c, c, i);
    fdtests[i] = open(portname, O_RDWR | O_BINARY);
    printf("fdtest[%d]: %d\n", i, fdtests[i]);
    if (fdtests[i] > 0) {
      fd[numopen] = fdtests[i];
      //flags = fcntl(fd[numopen], F_GETFL, 0);
      //fcntl(fd[numopen], F_SETFL, flags | O_NONBLOCK);
      numopen = numopen + 1;
    }
  }

  if (!numopen) {
    printf("Error in scanning for ports, exiting program\n");
    return(0);
  }

  printf("Number of connected subcontrollers:%d\n", numopen);
  free(portname);
  free(fdtests);

  //================Setup Serial Comms With Subcontrollers=======--==
  for (i = 0; i < numopen; i++) {
    configtty(fd[i], 115200);
  }

  printf("here\n");

  //======================Start Handshake=================
  char * hsbuf = malloc(2);
  mpause(999);
  mpause(999);
  //mpause(999);


  unsigned char * speriod = malloc(3);
  speriod[0] = 0;
  speriod[1] = 0;
  speriod[2] = 0;

  int * periods = malloc(sizeof(int)*(numopen + 1));

  unsigned char * numsen = malloc(2);
  numsen[0] = 0;
  numsen[1] = 0;

  int * sens = malloc(sizeof(int)*(numopen + 1));



  for (i = 0; i < numopen; i++) {
    printf("Init Handshake %d\n", i);
    write(fd[i], "s", 1);

    while (1) {
      printf("stucc\n");
      fflush(stdout);
      //mpause(5);
      if (read(fd[i], hsbuf, 1)) {
	if (hsbuf[0] == 'b') {
	  printf("Sent and received from arduino, finishing handshake\n");
	  fflush(stdout);
	  write(fd[i], "m", 1);

	  read(fd[i], speriod, 2);
	  read(fd[i], numsen, 1);
	  periods[i] = speriod[0] + speriod[1] * 256;
	  sens[i] = numsen[0];
	  printf("%d: period: %d sens: %d:\n", i, periods[i], sens[i]);

	  break;
	}
      }
    }
  }


  //=====================Data Collected=============
  //strt
  int periodmax = 0;

  for (i = 0; i < numopen; i++) {
    if (periodmax < periods[i]) {
      periodmax = periods[i];
    }
  }

  int * bufsizs = malloc(sizeof(int)*numopen + 1);
  int databufsiz = 0;
  int packsize = 6;

  for (i = 0; i < numopen; i++) {
    bufsizs[i] = sens[i] * (int)(periodmax / periods[i]) * packsize;
    databufsiz = databufsiz + bufsizs[i];
  }

  int maxbufsiz = 0;

  for (i = 0; i < numopen; i++) {
    if (maxbufsiz < bufsizs[i]) {
      maxbufsiz = bufsizs[i];
    }
  }


  unsigned char * databuf = malloc(databufsiz+1);
  unsigned char * dummybuf = malloc(maxbufsiz+1);


  for (i = 0; i < databufsiz+1; i++) {
    databuf[i] = 0;
  }

  for (i = 0; i < maxbufsiz+1; i++) {
    dummybuf[i] = 0;
  }

  int tmr = 0;

  int * readsiz = malloc(sizeof(int)*(numopen + 1));
  int * readtot = malloc(sizeof(int)*(numopen + 1));

  for (i = 0; i < numopen + 1; i++) {
    readsiz[i] = 0;
    readtot[i] = 0;
  }

  //int timinit = (int)current_time;
  //int timnew = (int)current_time;

  struct timeb timinit, timnew;
  ftime(&timinit);
  ftime(&timnew);

  while (1) {
    //read once from all streams
    //mpause(125);
    for (i = 0; i < numopen; i++) {
      fflush(stdout);

      printf("\n%d Start Read\n", i);

      for (k = 0; k < numopen; k++) {
	printf("i: %d, k: %d, databuf:", i, k);
	for (j = 0; j < packsize; j++) {
	  printf("%d, ", databuf[k*sens[k]*packsize + j]);
	}
	printf("\n");
      }
      //printf("\n");
      printf("%d readsiz: %d readtot: %d\n", i, readsiz[i], readtot[i]);
      readsiz[i] = read(fd[i], dummybuf, bufsizs[i] - readtot[i]);
      printf("%d readsiz: %d readtot: %d pc-r: %d\n", i, readsiz[i], readtot[i], bufsizs[i] - readtot[i]);
      printf("i: %d, dummybuf:", i);
      for (j = 0; j < maxbufsiz; j++) {
	printf("%d, ", dummybuf[j]);
      }
      printf("\n");

      //if anything was read, increment the total number of bytes read
      if (readsiz[i] > 0) {
	for (k = 0; k < readsiz[i]; k++) {
	  databuf[i*packsize*sens[i] + k + readtot[i]] = dummybuf[k];
	}
	readtot[i] = readtot[i] + readsiz[i];
      }

      printf("%d readsiz: %d readtot: %d\n", i, readsiz[i], readtot[i]);

      for(k=0;k<numopen;k++){
	printf("i: %d, k: %d, databuf:", i,k);
	for(j=0;j<packsize;j++){
	  printf("%d, ",databuf[k*sens[k]*packsize+j]);
	}
	printf("\n");
      }
      //printf("\n");



      //reconstruct and store the data
      if (bufsizs[i] - readtot[i] == 0) {
	for(k=0;k<(bufsizs[i])/packsize;k++){
	  printf("Read from sc %d\n", i);
	  fullsen[0] = databuf[(i*sens[i]+k)*packsize] + i * 100;
	  fulltim[0] = databuf[(i*sens[i]+k)*packsize + 1] + databuf[(i*sens[i]+k)*packsize + 2] * 256 + databuf[(i*sens[i]+k)*packsize + 3] * 256 * 256;
	  fullmsg[0] = databuf[(i*sens[i]+k)*packsize + 4] + databuf[(i*sens[i]+k)*packsize + 5] * 256;
	  dat[0] = fulltim;
	  dat[1] = fullsen;
	  dat[2] = fullmsg;

	  csvappend(fpath, cols, 1, dat);
	}

	readsiz[i] = 0;
	readtot[i] = 0;

	ftime(&timnew);
	printf("Time Taken: %d\n", timnew.millitm-timinit.millitm);
	ftime(&timinit);

	//timnew = (int)current_time;
	//printf("Time Taken: %d\n", timnew - timinit);
	//timinit = timnew;
      }
    }
    //timnew = (int)current_time;
    //printf("Time Taken: %d\n", timnew-timinit);
    //timinit = timnew;
    //printf("Time Taken: %lu\n", timediff);

  }

  //===================Free Allocated Memory===============

  free(dummybuf);
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
