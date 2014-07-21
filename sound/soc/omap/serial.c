#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
void delay(int tt)
{
  int i;
  for(;tt>0;tt--)
  {
	for(i=0;i<10000;i++){}
  }
}

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
  struct termios newtio,oldtio;
  if  ( tcgetattr( fd,&oldtio)  !=  0) { 
	perror("SetupSerial 1");
	return -1;
  }
  bzero( &newtio, sizeof( newtio ) );
  newtio.c_cflag  |=  CLOCAL | CREAD; 
  newtio.c_cflag &= ~CSIZE; 

  switch( nBits )
  {
	case 7:
	  newtio.c_cflag |= CS7;
	  break;
	case 8:
	  newtio.c_cflag |= CS8;
	  break;
  }

  switch( nEvent )
  {
	case 'O':
	  newtio.c_cflag |= PARENB;
	  newtio.c_cflag |= PARODD;
	  newtio.c_iflag |= (INPCK | ISTRIP);
	  break;
	case 'E': 
	  newtio.c_iflag |= (INPCK | ISTRIP);
	  newtio.c_cflag |= PARENB;
	  newtio.c_cflag &= ~PARODD;
	  break;
	case 'N':  
	  newtio.c_cflag &= ~PARENB;
	  break;
  }

  switch( nSpeed )
  {
	case 2400:
	  cfsetispeed(&newtio, B2400);
	  cfsetospeed(&newtio, B2400);
	  break;
	case 4800:
	  cfsetispeed(&newtio, B4800);
	  cfsetospeed(&newtio, B4800);
	  break;
	case 9600:
	  cfsetispeed(&newtio, B9600);
	  cfsetospeed(&newtio, B9600);
	  break;
	case 115200:
	  cfsetispeed(&newtio, B115200);
	  cfsetospeed(&newtio, B115200);
	  break;
	default:
	  cfsetispeed(&newtio, B9600);
	  cfsetospeed(&newtio, B9600);
	  break;
  }
  if( nStop == 1 )
	newtio.c_cflag &=  ~CSTOPB;
  else if ( nStop == 2 )
	newtio.c_cflag |=  CSTOPB;
  newtio.c_cc[VTIME]  = 0;
  newtio.c_cc[VMIN] = 0;
  tcflush(fd,TCIFLUSH);
  if((tcsetattr(fd,TCSANOW,&newtio))!=0)
  {
	perror("com set error");
	return -1;
  }
  printf("set done!\n");
  return 0;
}

int open_port(int fd,int comport)
{
  char *dev[]={"/dev/tts/0","/dev/tts/1","/dev/tts/2"};
  long  vdisable;
  if (comport==0)
  {	
	fd = open( "/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
	  perror("Can't Open Serial Port0");
	  return(-1);
	}
	else 
	  printf("open tts/0 .....\n");
  }
  else if(comport==1)
  {	
	fd = open( "/dev/ttyS1", O_RDWR/*|O_NOCTTY|O_NDELAY*/);
	if (-1 == fd){
	  perror("Can't Open Serial Port2");
	  return(-1);
	}
	else 
	  printf("open tts/1 .....\n");
  }
  else if (comport==2)
  {
	fd = open( "/dev/ttyS2", O_RDWR/*|O_NOCTTY|O_NDELAY*/);
	if (-1 == fd){
	  perror("Can't Open Serial Port3");
	  return(-1);
	}
	else 
	  printf("open tts/2 .....\n");
  }
  if(fcntl(fd, F_SETFL, 0)<0)
	printf("fcntl failed!\n");
  else
	printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
  if(isatty(STDIN_FILENO)==0)

	printf("standard input is not a terminal device\n");
  else
	printf("isatty success!\n");
  printf("fd-open=%d\n",fd);
  return fd;
}

int open_record()
{
	char command[256];
	int result;
	sprintf(command,"%s","/usr/local/alsa/bin/amixer -c 0 sset \'Analog Right Sub Mic\' cap");
	printf("open_record %s\r\n",command);
	result=system(command);
	printf("open_record result %d\r\n",result);
	return result;
}
int main(int argc,char *argv[])
{
  int fd;
  int nread,nwrite,i,j;
  char buff0[3]="ATD";
  char buff1[3]="ATA";
  char buff2[7]="AT+CHUP";
  char end='\r';
  char end_sent=';';

  printf("Port %d,Phone %s\r\n",argv[1][0]-48,argv[2]);	
  open_record();
  if((fd=open_port(fd,(int)(argv[1][0]-48)))<0){
	perror("open_port error");
	return;
  }
  if((i=set_opt(fd,115200,8,'N',1))<0){
	perror("set_opt error");
	return;
  }
  printf("fd=%d\n",fd);
  if(argc==2)
  {/*receive call*/
	printf("accepting call from No.%s 3G module\r\n",argv[1]);
	write(fd,buff1,3);
	write(fd,&end,1);
  }
  else
  {/*sent call*/
	printf("call to %s through No.%s 3G module\r\n",argv[2],argv[1]);
	write(fd,buff2,7);
	write(fd,&end,1);
	sleep(1);
	write(fd,buff0,3);
	write(fd,argv[2],strlen(argv[2]));
	write(fd,&end_sent,1);
	write(fd,&end,1);
  }
  close(fd);
  return;
}

