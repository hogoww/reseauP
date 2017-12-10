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

void* f2(void* v){
  sleep(5);
  printf("something\n");
  pthread_exit(NULL);
}

void* f(void* v){   
  int i;
  for(i=0;i<10;i++){
    pthread_t t;
    pthread_create(&t,NULL,f2,NULL);
  }
  sleep(3);
  printf("somethingelse\n");
  pthread_exit(NULL);
}


int main(){
  pthread_t t;
  pthread_create(&t,NULL,f,NULL);
  printf("created\n");
  sleep(1);
  printf("done sleeping\n");
  pthread_cancel(t);
  printf("done\n");
  sleep(10);
  return 0;
}

