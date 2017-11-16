/* gcc -Wall -ansi -pedantic -std=c99 -o serveur serveur.c */

#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#define SIZE_BUFF 4096

static int receiving=0;
int sockListen;
int descSocketListen;
int descClient;
FILE* fileToReceive;  
char* buffer;



void sigINT_handler(int signo){

  if(signo == SIGINT){
    if(receiving){
      printf("kill me when i'm working ain't nice!\n");
      printf("closing though\n");
      close(descClient);
      fclose(fileToReceive);
    }
    else{
      printf("Closing, have a nice day :)\n");
    }
    close(sockListen);
    close(descSocketListen);
    free(buffer);
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
  
  
  descSocketListen=bind(sockListen,(struct sockaddr*)&addr,sizeof(struct sockaddr));
  if(-1==descSocketListen){
    fprintf(stderr,"problème bind : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }
  if(-1==listen(sockListen,1)){
    fprintf(stderr,"problème listen : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }
  

  /* struct stat s; */
  /* checkReturn=stat(argv[3],&s); */
  /* s.st_size*/


  while(1){
    struct sockaddr_in client;
    socklen_t sizeClient=sizeof(client);
    
    printf("waiting for a new client\n");
    descClient=accept(sockListen,(struct sockaddr*)&client,&sizeClient);
    if(-1==descClient){
      fprintf(stderr,"problème accept : %s.\n",strerror(errno));
      exit(1);
    }

    int c=fork();
    if(c==-1){
      fprintf(stderr,"problème fork : %s.\n",strerror(errno));
      exit(EXIT_FAILURE);
    }
    else{
      if(c!=0){//father
	close(descClient);
      }
      else{//son
	close(sockListen);
	close(descSocketListen);

	printf("receiving a file");

	buffer=malloc(sizeof(char)*SIZE_BUFF);

	ssize_t res=recv(descClient,buffer,SIZE_BUFF,0);
	if(-1==res){
	  fprintf(stderr,"problème recv fileName : %s.\n",strerror(errno));
	  free(buffer);
	  exit(EXIT_FAILURE);
	}

	int j=0;
	while(buffer[j]!='\0'){
	  j++;
	}
    
	char* fileName=malloc(sizeof(char)*j);    
    
	ssize_t i=0;
	while(buffer[i]!='\0'){
	  fileName[i]=buffer[i];
	  i++;
	}
    
    
	printf("\n%s\n",fileName);
      
	fileToReceive=fopen(fileName,"w");
	if(fileToReceive==NULL){
	  fprintf(stderr,"file failed to open : %s.\n",strerror(errno));
	  exit(EXIT_FAILURE);
	}

	i++;
	while(i<=res && buffer[i]!='\0'){
	  fputc(buffer[i],fileToReceive);
	  i++;
	}

	while(1){
	  ssize_t res=recv(descClient,buffer,SIZE_BUFF,0);
	  if(-1==res){
	    fprintf(stderr,"problème recv : %s.\n",strerror(errno));
	    close(descClient);
	    close(sockListen);
	    fclose(fileToReceive);
	    free(buffer);
	    exit(EXIT_FAILURE);
	  }

      
	  if(res!=0){
	    if(res!=SIZE_BUFF && buffer[res-1]=='\0'){
	      res--;
	    }

	    ssize_t r=fwrite(buffer,1,res,fileToReceive);
	    if(res!=r){
	      printf("weird write\n");
	    }
	  }

	  if((res!=SIZE_BUFF && buffer[res-1]=='\0') || res==0){
	    break;
	  }
      
	  //write 
	}
	close(descClient);
	free(buffer);
	fclose(fileToReceive);
	printf("done receiving a file");
	return 0;
      }
    }
  }
    
  close(sockListen);
  close(descSocketListen);  

  free(buffer);
  
  return 0;
}
