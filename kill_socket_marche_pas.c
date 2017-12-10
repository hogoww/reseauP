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
#include <pthread.h>

int sockListen;
int descClient;

void* f(void* v){   
  int r=errno;
  printf("waiting\n");
  accept(sockListen,NULL,NULL);
  printf("done waiting\n");
  int r1=errno;
  printf("pré=%d post=%d current=%d: %s\n",r,r1,errno,strerror(errno));
  pthread_exit(NULL);
}


int main(){
  sockListen=socket(PF_INET,SOCK_STREAM,0);//création de la socket

  struct sockaddr_in addr;//et remplissage
  
  memset(&addr,0,sizeof(addr));
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=INADDR_ANY;
  addr.sin_port=htons(12000);
  
  
  int checkBind=bind(sockListen,(struct sockaddr*)&addr,sizeof(struct sockaddr));//binding de la socket.
  if(-1==checkBind){
    if(errno==EADDRINUSE){
      fprintf(stderr,"L'OS ne tolère pas que le port du socket soit identique entre deux exécutions proches.\nPour plus d'informations sur pourquoi cette attente : \n\nhttps://stackoverflow.com/questions/775638/using-so-reuseaddr-what-happens-to-previously-open-socket\n\n");
    }
    else{
      fprintf(stderr,"problème bind : %d %s.\n",errno,strerror(errno));
    }
    exit(EXIT_FAILURE);
  }
  
  if(-1==listen(sockListen,1)){//pour mettre la socket en listener
    fprintf(stderr,"problème listen : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }

  pthread_t t;
  pthread_create(&t,NULL,f,NULL);
  printf("created\n");
  sleep(1);
  printf("done sleeping\n");
  printf("closing %d\n",close(sockListen));
  pthread_join(t,NULL);
  printf("done\n");
  return 0;
}

