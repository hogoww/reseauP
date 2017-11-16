// gcc -Wall -ansi -pedantic -std=c99 -o server server.c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/select.h>

#define SIZE_BUFF 100
#define MAX_NB_CLIENT 10

char* buffer;
int sockListen;
int test;
int max;
fd_set towrite;

void sigINT_handler(int signo){
  if(signo == SIGINT){
    buffer[0]='1';
    buffer[1]='5';
    buffer[2]='4';
    buffer[3]='2';
    for(int fd=2;fd<=max;++fd){
      printf("%d\n",fd);
      if(FD_ISSET(fd,&towrite)){	
	send(fd,buffer,4,0);
      }
    }
    free(buffer);
    close(sockListen);
    printf("shutting down nicely, have a nice day :)\n");
    exit(EXIT_SUCCESS);
  }
}

int main(int argc,char** argv){

  if(argc!=2){
    fprintf(stderr,"usage : ./main port\n");
    exit(EXIT_FAILURE);
  }

  if(signal(SIGINT, sigINT_handler) == SIG_ERR){
    fprintf(stderr,"Pas réussi à attribuer au signal INT sa trap\n");
    exit(EXIT_FAILURE);
  }

  sockListen=socket(PF_INET,SOCK_STREAM,0);

  struct sockaddr_in addr;
  memset(&addr,0,sizeof(addr));
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=INADDR_ANY;
  addr.sin_port=htons(atoi(argv[1]));
  
  test=bind(sockListen,(struct sockaddr*)&addr,sizeof(struct sockaddr));
  if(test==-1){
    fprintf(stderr,"problème bind : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }

  if(-1==listen(sockListen,1)){
    fprintf(stderr,"problème listen : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  buffer=malloc(sizeof(char)*SIZE_BUFF);

  fd_set set,settemp;
  FD_ZERO(&set);
  FD_ZERO(&towrite);
  FD_SET(sockListen,&set);

  max=sockListen;

  while(1){
    settemp=set;
    select(max+1,&settemp,NULL,NULL,NULL);
    for(int fd=2;fd<=max;++fd){

      if(!FD_ISSET(fd,&settemp)){//Nothing happened on that file descriptor
	continue;
      }
      printf("something happened on %d\n",fd);
      
      if(fd==sockListen){//Something appened on the listen socket
	struct sockaddr_in client;
	socklen_t sizeClient=sizeof(client);
	int descClient=accept(sockListen,(struct sockaddr*)&client,&sizeClient);
	if(-1==descClient){
	  fprintf(stderr,"problème accept : %s.\n",strerror(errno));
	  exit(EXIT_FAILURE);
	}
	else{
	  printf("accepting new client\n");
	  FD_SET(descClient,&set);
	  FD_SET(descClient,&towrite);
	  if(descClient>max){
	    max=descClient;
	  }
	}
	continue;
      }

      //Else, they can only be a reception to handle
      printf("receptionning\n");

      ssize_t res=recv(fd,buffer,SIZE_BUFF,0);
      if(-1==res){
	fprintf(stderr,"problème recv : %s.\n",strerror(errno));
	exit(EXIT_FAILURE);
      }
      
      printf("%s\n",buffer);

      if(buffer[0]=='q' && buffer[1]=='u' && buffer[2]=='i' && buffer[3]=='t'){//client wants to leave
	printf("client leaving :(\n");
	FD_CLR(fd,&set);
	FD_CLR(fd,&towrite);
	
	close(fd);
	continue;
      }

      //Show the message to ALL clients.
      for(int fd2=3;fd2<=max;++fd2){
	if(!FD_ISSET(fd2,&towrite) || fd2==fd){
	  continue;
	}
	//printf("iaze");
	ssize_t res2=send(fd2,buffer,res,0);
	if(-1==res2){
	  fprintf(stderr,"problème send : %s.\n",strerror(errno));
	  exit(EXIT_FAILURE);
	}
      }
    }
  }
  close(sockListen);
  free(buffer);
  return 0;
}




