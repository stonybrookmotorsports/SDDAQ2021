#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include "datareader.h"
#include "filewriter.h"
#include "aux.h"


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

  DCB dcbSerialParams = { 0 }; // Initializing DCB structure
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  dcbSerialParams.BaudRate = CBR_115200;  // Setting BaudRate = 9600
  dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
  dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
  dcbSerialParams.Parity = NOPARITY;  // Setting Parity = None

  return 0;
  
  /*
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
  */
}


