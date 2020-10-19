#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "datareader.h"
#include "filewriter.h"

/*=============================
pause makes the system wait for some value millis=(0,1000) milliseconds
=============================*/
void mpause(int millis){
  struct timespec ts, ts2;
  ts.tv_sec=0;
  ts.tv_nsec = millis * 1000000L;
  printf("%d\n", nanosleep(&ts,&ts2));
  printf("%d\n", errno);
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
  for(k = 0; k < colsiz; k++){
    fulltim[k] = 1;
    fullmsg[k] = 2;
  }
  
  //=====================Real CSV Hours out here===================
  int cols = 2;
  char ** names = malloc(colsiz*cols);//size arbitrary
  int ** dat = malloc(2*sizeof(int));
  printf("Writing to CSV \n");
  fflush(stdout);
    
  names[0] = "time";
  names[1] = "rpm";

  dat[0] = fulltim;
  dat[1] = fullmsg;
  
  csvwrite(fpath, cols, colsiz, names, dat, nmwrt);

  //===============Startup and portnumber=================
  char * c = malloc(2);
  c[0] = 0;
  c[1] = 0;
  
  printf("Welcome to SBMDAQ 1.0.0, please enter a value between 0-9 of the COM (windows) or TTYACM (Ubuntu/MAC) port that you will be utilizing:\n");

  while(1){
    c[0] = getchar();
    mpause(25);
    //follow up with - 48 for ascii code adjust
    if(c[0] - 48 >= 0 && c[0] - 48 < 10){
      printf("\nOpening Port: %d\n", c[0] - 48);
      break;
    }
    else{
      printf("\nError in port number, please re-enter the value and make sure the value is between 0-9:\n");
    }
  }


  //====================Opening Port=======================
  char *portname = malloc(15);
  int iter = 0;
  for(;iter < 15; iter++){
    portname[0] = 0;
  }
  int fd;
  int wlen;
  int exec = 0;
  pid_t pid;
  int status;
  
  strcat(portname, "/dev/ttyACM");
  strcat(portname, c);

  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    printf("Error opening %s: %s\n", portname, strerror(errno));
    return -1;
  }
  free(portname);
  free(c);

  /*baudrate 115200, 8 bits, no parity, 1 stop bit */
  configtty(fd, B115200);


  //======================Start Handshake=================
  char * hsbuf = malloc(2);
  mpause(999);
  mpause(999);
  mpause(999);

  printf("Init Handshake\n");
  write(fd, "s", 1);  
  
  while(1){
    printf("Querying\n");
    fflush(stdout);
    if(read(fd, hsbuf, 1)){
      if(hsbuf[0] == 'b'){
	printf("Sent and received from arduino, finishing handshake\n");
	fflush(stdout);
	write(fd, "m", 1);
	break;
      }
    }
  }
  
  csvappend(fpath, cols, colsiz, dat);
  csvappend(fpath, cols, colsiz, dat);
  csvappend(fpath, cols, colsiz, dat);
  
  free(names);
  free(dat);
  free(fulltim);
  free(fullmsg);
}
