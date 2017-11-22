#include <unistd.h>/*chdir*/
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string.h>
#include <errno.h>

#define SIZE_BUFF 4096

struct servParam{
  /*TO DO*/
};


int ConnectToServ(char* servAdress/*argv[1]*/){
  int sockListener=socket(PF_INET,SOCK_STREAM,0);//création de la socket
  if(sockListener==-1){fprintf(stderr,"Problem init ConnectToserv socket : %s.\n",strerror(errno));exit(EXIT_FAILURE);}
  
  struct sockaddr_in addr;//et remplissage des infos passé en ligne de commande.
  memset(&addr,0,sizeof(addr));
  addr.sin_family=AF_INET;
  int checkReturn=inet_pton(AF_INET,servAdress,&(addr.sin_addr));
  if (checkReturn == 0){
    fprintf(stderr, "Adresse fournie format incorrect");
    close(sock);
    exit(EXIT_FAILURE);
  }
  addr.sin_port=htons(atoi(argv[2]));

}

int main(int argc,char** argv){
  int portAnnuaire=0;
  if(argc>2){/*Check arguments*/
    fprintf(stderr,"\n\nusage : ./serveur portAnnuaire\n\n");
    exit(EXIT_SUCCESS);
  }
  if(argc==2){
    portAnnuaire=htons(atoi(argv[1]));
  }
  else{
    portAnnuaire=12000;/*Codé en dur pour debug*/
  }


  
  
  

  return 0;
}
