/*classique*/
#include <unistd.h>/*chdir*/
#include <stdio.h>
#include <stdlib.h>

/*includes fonctions reaseau*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*erreur*/
#include <string.h>
#include <errno.h>

/*Manipulation dossier*/
#include <dirent.h>

#define QUERY_BUFF_SIZE
#define SIZE_BUFF 4096
#define SERV_ADDRESS "127.0.0.1"

#define LEAVING '1'
#define COMING '2'
#define REFRESH '3'


struct servParam{
  int i;
  /*TO DO*/
};

/*get peer name*/
/******************************************/
/*MODIFY PARAMETER TO INCLUDE SERV ADDRESS*/
/******************************************/
/*Connection annuaire | pair*/
int ConnectToServ(int port);
void DisconnectFromServ(int sock);

/*interaction Annuaire - pair*/
void DisAuServeurQueJeSuisPresent(int port,char* pseudo,int size_P);
void DisAuServeurQueJeQuitte(int port,char* pseudo,int size_P);
void RefreshThatList(int port);/*Todo*/

void QueryTypeServ(int serv,char type);


/*Manipulation*/
int LookForEnd(char s[],int size);
char* ConvertBracketToStar(char s[],int fin,int size);
int CharPToInt(char* c,int size);



int main(int argc,char** argv){
  if(argc!=3){/*Check arguments*/
    fprintf(stderr,"\n\nusage : ./p [Mon_Pseudo] [Port_Annuaire] \n\n");
    exit(EXIT_SUCCESS);
  }
  int port=htons(atoi(argv[2]));
  char* pseudo=argv[1];
  int size_P=LookForEnd(argv[1],1000);/*Viens de bash, donc seras bien formaté*/

  DisAuServeurQueJeSuisPresent(port,pseudo,size_P);
  
  int done=0;
  do{
    printf("\nR -> Refresh la liste de l'annuaire\nQ-> Quite le reseau\n");

    switch(fgetc(stdin)){
    case 'R':
      break;
    case 'Q':
      done=1;
      break;
    default:
      printf("Commande non reconnue\n");
      break;
    }

  }while(!done);
  
  DisAuServeurQueJeQuitte(port,pseudo,size_P);

  return 0;
}

int ConnectToServ(int port){
  int sock=socket(PF_INET,SOCK_STREAM,0);//création de la socket
  if(sock==-1){fprintf(stderr,"Probleme ConnectToserv socket() : %s.\n",strerror(errno));exit(EXIT_FAILURE);}
  
  struct sockaddr_in addr;//et remplissage des infos passé en ligne de commande.
  memset(&addr,0,sizeof(addr));
  addr.sin_family=AF_INET;
  int checkReturn=inet_pton(AF_INET,SERV_ADDRESS,&(addr.sin_addr));
  if (checkReturn==0 || checkReturn==-1){
    fprintf(stderr, "ConnectToServ : Adresse fournie format incorrect ; %s.\n",strerror(errno));
    DisconnectFromServ(sock);
    exit(EXIT_FAILURE);
  }
  addr.sin_port=htons(port);

  if(-1==connect(sock,(struct sockaddr*)&addr,sizeof(struct sockaddr))){//Connection au serveur
    fprintf(stderr,"problème ConnectToServ Connect() : %s.\n",strerror(errno));
    DisconnectFromServ(sock);
    exit(EXIT_FAILURE);
  }
  
  return sock;
}

void DisconnectFromServ(int sock){
  if(-1==close(sock)){fprintf(stderr,"Probleme DisconnectToServ socket() : %s.\n",strerror(errno));}
}

int LookForEnd(char s[],int size){
  int i=0;
  while(i<size && s[i]!='\0'){++i;}
  return i;
}

char* ConvertBracketToStar(char s[],int end,int size){
  if(end>size){
    fprintf(stderr,"Convert char[] to char* failed\n");
  }
  int i=0;
  char* res=malloc(sizeof(char)*end+1);
  if(res==NULL){fprintf(stderr,"Problème Malloc : %s.\n",strerror(errno));exit(EXIT_FAILURE);}
  while(i<end){
    res[i]=s[i];
    i++;
  }
  res[i]='\0';
  return res;
}


void DisAuServeurQueJeSuisPresent(int port,char* pseudo,int size_P){
  int serv=ConnectToServ(port);


  SendPseudo(serv,pseudo,size_P);
  
  DIR* d;
  struct dirent* dir;
  d=opendir(".");
  if(d){
    while ((dir = readdir(d)) != NULL){
      int end=LookForEnd(dir->d_name,256);
      char* r=ConvertBracketToStar(dir->d_name,end,256);
      res=send(serv,r,end,0);//Envois du nom du fichier au serveur.
      if(-1==res){
	fprintf(stderr,"DisAuServeurQueJeSuisPresent() problème envoie d'un nom de fichier : %s.\n",strerror(errno));
	closedir(d);
	DisconnectFromServ(port);
	exit(EXIT_FAILURE);
      }
      free(r);
    }

    res=send(serv,pseudo,0,0);//fin des fichiers à réceptionner.
    if(-1==res){
      fprintf(stderr,"problème DisAuServeurQueJeSuisPresent() envoie signal fin de fichier : %s.\n",strerror(errno));
      closedir(d);
      DisconnectFromServ(serv);    
      exit(EXIT_FAILURE);
    }

    closedir(d);
  }
  else{
    fprintf(stderr,"DisAuServeurQueJeSuisPresent Le repertoire n'as pas été ouvert : %s.\n",strerror(errno));
    DisconnectFromServ(serv);
    exit(EXIT_FAILURE);
  }

  DisconnectFromServ(serv);
}
  

void QueryTypeServ(int serv,char type){
  char* e=malloc(sizeof(char));
  *e=type;
  ssize_t res=send(serv,pseudo,size_P,0);//Envois du nom du fichier au serveur.
  if(-1==res){
    fprintf(stderr,"problème DisAuServeurQueJeQuitte() send pseudo : %s.\n",strerror(errno));
    DisconnectFromServ(serv);    
    exit(EXIT_FAILURE);
  }
  free(e);
}

void SendPseudo(int serv,char* pseudo,int size_P){

  ssize_t res=send(serv,pseudo,size_P,0);//Envois du nom du fichier au serveur.
  if(-1==res){
    fprintf(stderr,"problème DisAuServeurQueJeQuitte() send pseudo : %s.\n",strerror(errno));
    DisconnectFromServ(serv);    
    exit(EXIT_FAILURE);
  }
}

void DisAuServeurQueJeQuitte(int port,char* pseudo,int size_P){
  int serv=ConnectToServ(port);  
  
  QueryTypeServ(serv,LEAVING);
  SendPseudo(serv,pseudo,size_P);
  
  DisconnectFromServ(serv);
}

int CharPToInt(char* c,int size){
  int end=LookForEnd(c,size);
  char* res=malloc(sizeof(char)*end+1);
  int i=0;
  while(i<end){
    res[i]=c[i];
    ++i;
  }
  res[i]='\0';
  return atoi(res);
}


void RefreshThatList(int port){
  int serv=ConnectToServ(port);
  QueryTypeServ(serv,REFRESH);

  char* buffer=malloc(sizeof(char)*SIZE_BUFF);

  ssize_t res=recv(serv,buffer,SIZE_BUFF,0);//Reception du nom
  if(-1==res){
    fprintf(stderr,"problème recv fileName : %s.\n",strerror(errno));
    free(buffer);
    DisconnectFromServ(serv);
    exit(EXIT_FAILURE);
  }

  CharPToInt(buffer,SIZE_BUFF);
  
  res=recv(serv,buffer,SIZE_BUFF,0);//Reception du nom
  if(-1==res){
    fprintf(stderr,"problème recv fileName : %s.\n",strerror(errno));
    free(buffer);
    DisconnectFromServ(serv);
    exit(EXIT_FAILURE);
  }
  
  
  res=recv(serv,buffer,SIZE_BUFF,0);//Reception du nom
  if(-1==res){
    fprintf(stderr,"problème recv fileName : %s.\n",strerror(errno));
    free(buffer);
    DisconnectFromServ(serv);
    exit(EXIT_FAILURE);
  }

  DisconnectFromServ(serv);
}



/* /\******************************************\/ */
/* /\*MODIFY PARAMETER TO INCLUDE SERV ADDRESS*\/ */
/* /\******************************************\/ */
/* int ConnectToServ(int port){ */
/*   int sockListener=socket(PF_INET,SOCK_STREAM,0);//création de la socket */
/*   if(sockListener==-1){fprintf(stderr,"Probleme ConnectToserv socket() : %s.\n",strerror(errno));exit(EXIT_FAILURE);} */
  
/*   struct sockaddr_in addr;//et remplissage des infos passé en ligne de commande. */
/*   memset(&addr,0,sizeof(addr)); */
/*   addr.sin_family=AF_INET; */
/*   int checkReturn=inet_pton(AF_INET,SERV_ADDRESS,&(addr.sin_addr)); */
/*   if (checkReturn == 0){ */
/*     fprintf(stderr, "Adresse fournie format incorrect"); */
/*     close(sock); */
/*     exit(EXIT_FAILURE); */
/*   } */
/*   addr.sin_port=htons(port); */

/*   int check=bind(sockListen,(struct sockaddr*)&addr,sizeof(struct sockaddr));//binding de la socket.   */
/*   if(check==-1){fprintf(stderr,"Probleme ConnectToserv bind() : %s.\n",strerror(errno));exit(EXIT_FAILURE);} */

/*   if(-1==listen(sockListen,1))//pour mettre la socket en passif. */
/*     {fprintf(stderr,"problème ConnectToserv listen() : %s.\n",strerror(errno));exit(EXIT_FAILURE);} */
  
/*   return sockListen; */
/* } */
