#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>

int g_fd_1,g_fd_2;
char phone_in[100][20];
char phone_out[100][20];
int phone_map_len=100;
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

int open_port(int comport)
{
	int fd;
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
	if(fcntl(fd, F_SETFL, FNDELAY)<0)
		printf("fcntl failed!\n");
	else
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,FNDELAY));
	if(isatty(STDIN_FILENO)==0)

		printf("standard input is not a terminal device\n");
	else
		printf("isatty success!\n");
	printf("fd-open=%d\n",fd);
	return fd;
}
int phone_process(int fd,int type,char *phone_number)
{
	int nread,nwrite,i,j;
	char buff0[3]="ATD";
	char buff1[3]="ATA";
	char buff2[7]="AT+CHUP";
	char buff3[9]="AT+CLIP=1";
	char end='\r';
	char end_sent=';';
	if(type==0)//call out
	{
		printf("call Phone %s through serial fd %d\r\n",phone_number,fd);	
		write(fd,buff0,3);
		write(fd,phone_number,strlen(phone_number));
		write(fd,&end_sent,1);
		write(fd,&end,1);
	}
	else if(type==1)//open call in display
	{
		printf("open incoming phone number display on fd %d\r\n",fd);	
		write(fd,buff3,9);
		write(fd,&end,1);
	}
	else if(type==2)//reject call in
	{
		printf("reject incoming call on fd %d\r\n",fd);	
		write(fd,buff2,7);
		write(fd,&end,1);
	}
	else if(type==3)//accept call in
	{
		printf("accept incoming call on fd %d\r\n",fd);	
		write(fd,buff1,3);
		write(fd,&end,1);
	}
	return 0;
}
int wait_phone_call(char **out)
{
	char buf[256],*buf2;
	char ch;
	int i=0,j=0;
	memset(buf,'\0',256);
	//check Calling in from 3G1
	while(read(g_fd_1,&ch,1)==1)
	{
		buf[i++]=ch;
	}
	if(i!=0)
	{
		buf[i]='\0';
		printf("3G in %s\r\n",buf);
		if(strncmp(buf,"\r\nRING",6)==0)
		{
			*out=(char *)malloc(20*sizeof(char));
			buf2=*out;  
			memset(buf2,'\0',20);
			i=0;
			while(buf[i]!='\"')
			{
				i++;
			}
			i++;
			while(buf[i]!='\"')
				buf2[j++]=buf[i++];
			buf2[j]='\0';
			printf("Calling in 3G1 %s\r\n",buf2);
			return 1;
		}
		else
			printf("no Ring in from 3g1\r\n");
	}
	//check Calling in from PSTN
	i=0;
	int fd_pstn = open( "/dev/cmx865a0", O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd_pstn){
		perror("Can't Open PSTN");
	}
	else 
		printf("open PSTN .....\n");
	while(read(fd_pstn,&ch,1)==1)
	{
		buf[i++]=ch;
	}
	close(fd_pstn);
	if(i!=0)
	{
		buf[i]='\0';
		*out=(char *)malloc(20*sizeof(char));
		buf2=*out;  
		memset(buf2,'\0',20);
		memcpy(*out,buf,strlen(buf));
		buf2[strlen(buf)]='\0';
		printf("Calling in PSTN %s\r\n",buf);
		return 2;
	}
	//check Calling in from VOIP
	
	return 0;
}
void print_system_status(int status)
{
	printf("status = %d\n",status);
	if(WIFEXITED(status))
	{
		printf("normal termination,exit status = %d\n",WEXITSTATUS(status));
	}
	else if(WIFSIGNALED(status))
	{
		printf("abnormal termination,signal number =%d%s\n",
		WTERMSIG(status),
#ifdef WCOREDUMP
		WCOREDUMP(status)?"core file generated" : "");
#else
		"");
#endif
	}
}
void kill_sound_task()
{
	DIR *dir;
	struct dirent *ptr;
	FILE *fp;
	char kill_cmd[256];
	char filepath[50];
	char cur_task_name[50];
	char buf[256];
	dir = opendir("/proc");
	if (NULL != dir)
	{
		while ((ptr = readdir(dir)) != NULL)
		{
			if((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))             
				continue;
			if(DT_DIR != ptr->d_type) 
				continue;
			sprintf(filepath, "/proc/%s/status", ptr->d_name);
			fp = fopen(filepath, "r");
			if (NULL != fp)
			{
				if( fgets(buf, 256-1, fp)== NULL )
				{
					fclose(fp);
					continue;
				}
				sscanf(buf, "%*s %s", cur_task_name);
				if (!strcmp("arecord", cur_task_name)||!strcmp("aplay", cur_task_name))
				{
					printf("PID:  %s\n", ptr->d_name);
					sprintf(kill_cmd,"kill -f %d",ptr->d_name);
					print_system_status(system(kill_cmd));
				}
				fclose(fp);
			}
		}
	closedir(dir);
	}
 }

int voice_route(int s,int t)
{
	char command[256];
	int result;
	kill_sound_task();
	memset(command,'\0',sizeof(char)*256);
	sprintf(command,"/usr/local/alsa/bin/arecord -D plughw:0,%d -r 8 -f S16_LE|/usr/local/alsa/bin/aplay -D plughw:0,%d&",s,t);
	printf("first to exec %s\r\n",command);
	print_system_status(system(command));
	sprintf(command,"/usr/local/alsa/bin/arecord -D plughw:0,%d -r 8 -f S16_LE|/usr/local/alsa/bin/aplay -D plughw:0,%d&",t,s);
	print_system_status(system(command));
	printf("second result %d\r\n",result);
	return 0;	
}
int open_record()
{
	char command[256];
	int result;
	sprintf(command,"%s","/usr/local/alsa/bin/amixer -c 0 sset \'Analog Right Sub Mic\' cap");
	printf("open_record %s\r\n",command);
	print_system_status(system(command));
	return result;
}
int main(int argc,char *argv[])
{
	int i=0,k=0,m=0,next_write=0;
	char ch;
	FILE *fp=NULL;
	char *in=NULL;
	printf("\r\nPhone System\r\n");
	open_record();
	if((g_fd_1=open_port(1))<0){
		perror("open_port error 1");
		return -1;
	}
	if(set_opt(g_fd_1,115200,8,'N',1)<0){
		perror("set_opt error 1");
		return -1;
	}
	if((g_fd_2=open_port(2))<0){
		perror("open_port error 2");
		return -1;
	}
	if(set_opt(g_fd_2,115200,8,'N',1)<0){
		perror("set_opt error 2");
		return -1;
	}
	for(i=0;i<phone_map_len;i++)
	{
		memset(phone_in[i],'\0',sizeof(char)*20);
		memset(phone_out[i],'\0',sizeof(char)*20);
	}
	fp=fopen("./phone.txt","r");
	if(fp<0)
	{
		perror("open phone.txt failed\r\n");
		return -1;
	}
	while(fread(&ch,sizeof(char),1,fp)==1)
	{
		if(ch==',')
		{/*next is route out phone number*/
			next_write=1;
			m=0;
		}
		else if(ch=='\n')
		{/*next is call in phone number*/
			next_write=0;
			k++;
			m=0;
		}
		else
		{/*store in phone_map*/
			if(next_write==1)
			{
				phone_out[k][m++]=ch;
			}
			else
			{
				phone_in[k][m++]=ch;
			}
		}
	}
	fclose(fp);
	k=k-1;
	for(i=0;i<k;i++)
	{
		printf("Phone[%d] in: %s <==> Phone[%d] out : %s\r\n",i,phone_in[i],i,phone_out[i]); 
	}
	phone_process(g_fd_1,1,NULL);
	//phone_process(g_fd_2,1,NULL);
	/*wait for phone call in */
	while(1)
	{
		int source=wait_phone_call(&in);
		if(source!=0)
		{
			for(i=0;i<k;i++)
			{
				printf("in %s <> phone_in list %s\r\n",in,phone_in[i]);
				if(strncmp(in,phone_in[i],strlen(in))==0)
				{
					printf("accept %s \r\nrout out %s\r\n",phone_in[i],phone_out[i]);
					phone_process(g_fd_2,3,NULL);//accept call
					phone_process(g_fd_1,0,phone_out[i]);//route to out call
					/*record voice from g_fd_2 , play to g_fd_1 */
					voice_route(source,0);
					break;
				}
				if(i==k-1)
				{
					printf("incoming call not in white list ,reject it\r\n");
					phone_process(g_fd_2,2,NULL);
				}
			}
			free(in);
			in=NULL;
		}
		else
		{
			sleep(1);
//			printf("No call in ,waiting...\r\n");
		}
	}
	return;
}

