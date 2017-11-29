/*classique*/
#include <unistd.h>/*chdir*/
#include <stdio.h>
#include <stdlib.h>

/*reseau*/
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

/*Threading*/
#include <pthread.h>

/*API personnelles*/
#include "listAssoc.h"
#include "shared_define.h"


#define SERV_ADDRESS "127.0.0.1"


struct servParam{
  int sock;
  /*TO DO*/
};
/*get peer name*/

/******************************************/
/*MODIFY PARAMETER TO INCLUDE SERV ADDRESS*/
/******************************************/

/*Connection*/
int ConnectToServ(uint16_t port);
void DisconnectFromServ(int sock);

/*interaction Annuaire - pair*/
void DisAuServeurQueJeSuisPresent(uint16_t port);
void DisAuServeurQueJeQuitte(uint16_t port);
void QueryTypeServ(int serv,char type);
struct listAssoc* RefreshThatList(uint16_t port);

/*Server side*/
int createServerSocket(uint16_t port);
void* IAMSERVEURNOW(void* param);
void IAMNOLONGERSERVER();

/*Manipulation*/
int LookForEnd(char s[],int size);
char* ConvertBracketToStar(char s[],int fin,int size);
int CharPToInt(char* c,int size);
int getIntFromServ(int serv);


int main(int argc,char** argv){
  if(argc!=2){/*Check arguments*/
    fprintf(stderr,"\n\nusage : ./p [Port_Annuaire] \n\n");
    exit(EXIT_FAILURE);
  }
  uint16_t port=htons(atoi(argv[1]));

  DisAuServeurQueJeSuisPresent(port);
  

  
  pthread_t server;/*Lancement serveur*/
  struct servParam *sp=malloc(sizeof(struct servParam));
  sp->sock=createServerSocket(port+1);/*Pour ne pas etre sur le meme port que le serveur annuaire*/
  if(-1==pthread_create(&server,NULL,IAMSERVEURNOW,(void*)sp)){
    fprintf(stderr,"problème creation thread serveur: %s.\n",strerror(errno));
  }


  struct listAssoc* list=NULL;
  list=RefreshThatList(port);
  DisplayListAssoc(list);
  int done=0;
  do{
    printf("\nR -> Refresh la liste de l'annuaire\nQ-> Quite le reseau\n");

    switch(fgetc(stdin)){
    case 'R':
      free(list);
      list=RefreshThatList(port);
      DisplayListAssoc(list);
      break;
    case 'Q':
      done=1;
      break;
    default:
      printf("Commande non reconnue\n");
      break;
    }

  }while(!done);

  delete_listAssoc_and_key_and_values(list);
  //free(list);
  IAMNOLONGERSERVER(sp->sock);
  printf("signal d'arret envoyé au serveur\n");
  DisAuServeurQueJeQuitte(port);
  printf("J'ai dis au serveur annuaire que je partais\n");

  printf("Et j'attends que le serveur soit terminé (il attends que les envois soit finis)\n");
  pthread_join(server,NULL);
  printf("serveur eteint, bonne journée:)\n");

  return 0;
}

int ConnectToServ(uint16_t port){
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

int getIntFromServ(int serv){

  char* e=malloc(sizeof(char)*32);/*max int = pow(2,32)*/
  ssize_t res=recv(serv,e,32,0);/*envoie le type de query sur le serv'*/
  if(-1==res){
    fprintf(stderr,"problème getIntFromServ : %s.\n",strerror(errno));
    DisconnectFromServ(serv);
    exit(EXIT_FAILURE);
  }
  int r=atoi(e);/*\0 doit avoit été mis correctement coté serv'.*/
  free(e);
  return r;
}

void DisAuServeurQueJeSuisPresent(uint16_t port){
  int serv=ConnectToServ(port);

  QueryTypeServ(serv,COMING);

  DIR* d;
  struct dirent* dir;
  d=opendir("./Seed");
  if(d){
    while ((dir = readdir(d)) != NULL){
      if((dir->d_name[0]='.')){
	continue;/*Pour ne pas afficher les fichier cachés*/
      }
      int end=LookForEnd(dir->d_name,256);
      char* r=ConvertBracketToStar(dir->d_name,end,256);
      ssize_t res=send(serv,r,end,0);//Envois du nom du fichier au serveur.
      
      if(-1==res){
	fprintf(stderr,"DisAuServeurQueJeSuisPresent() problème envoie d'un nom de fichier : %s.\n",strerror(errno));
	closedir(d);
	DisconnectFromServ(port);
	exit(EXIT_FAILURE);
      }
      free(r);
    }

    /*********************/
    /*CHECK IF THAT WORKS*/
    /*********************/
    ssize_t res=send(serv,NULL,0,0);//fin des fichiers à réceptionner.
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
  *e=(char)type;/*cast suffisant vu que le serveur le considèreras de la meme façon.*/
  ssize_t res=send(serv,e,1,0);/*envoie le type de query sur le serv'*/
  if(-1==res){
    fprintf(stderr,"problème DisAuServeurQueJeQuitte() queryTypeServ : %s.\n",strerror(errno));
    DisconnectFromServ(serv);    
    exit(EXIT_FAILURE);
  }
  free(e);
}


void DisAuServeurQueJeQuitte(uint16_t port){
  int serv=ConnectToServ(port);  
  
  QueryTypeServ(serv,LEAVING);
  
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

char* resize(char* s,int size){
  int end=LookForEnd(s,size);
  if(end>size){
    fprintf(stderr,"Didn't find a \\0 before i was out of char to check");
    exit(EXIT_FAILURE);
  }
  char* res=malloc(sizeof(char)*end+1);
  int i=0;
  while(i<end){
    res[i]=s[i];
    ++i;
  }
  res[i]='\0';
  return res;
}

struct listAssoc* RefreshThatList(uint16_t port){/*Not the most optimize, but that'll be enough*/
  int serv=ConnectToServ(port);

  QueryTypeServ(serv,REFRESH);

  int nbPair=getIntFromServ(serv);
  
  char* buffer=malloc(sizeof(char)*SIZE_BUFF);

  struct listAssoc* list=NULL;

  for(int i=0;i<nbPair;i++){
    ssize_t res=recv(serv,buffer,SIZE_BUFF,0);//Reception du nom du ième pair
    if(-1==res){
      fprintf(stderr,"problème recv pair name, RefreshThatList : %s.\n",strerror(errno));
      free(buffer);
      DisconnectFromServ(serv);
      exit(EXIT_FAILURE);
    }
    
    char* k=resize(buffer,SIZE_BUFF);
    if(!list){
      list=make_ListAssoc(k);
    }
    
    int nbFiles=getIntFromServ(serv);
   
    for(int j=0;j<nbFiles;j++){
      res=recv(serv,buffer,SIZE_BUFF,0);//Reception du jeme filename du ieme pair
      if(-1==res){
	fprintf(stderr,"problème recv fileName RefreshThatList : %s.\n",strerror(errno));
	free(buffer);
	DisconnectFromServ(serv);
	exit(EXIT_FAILURE);
      }
      char* fileName=resize(buffer,SIZE_BUFF);
      addValue_to_key_list(list,k,fileName);
    }
  }
  

  DisconnectFromServ(serv);
  return list;
}



int createServerSocket(uint16_t port){
  int sockListen=socket(PF_INET,SOCK_STREAM,0);//création de la socket
  
  struct sockaddr_in addr;//et remplissage
  memset(&addr,0,sizeof(addr));
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=INADDR_ANY;
  addr.sin_port=port;
  
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
  
  if(-1==listen(sockListen,1)){//pour mettre la socket en passif.
    fprintf(stderr,"problème listen : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  return sockListen;
}

void* IAMSERVEURNOW(void* param){
  struct servParam* p=(struct servParam*)param;
  
  char* buffer=malloc(sizeof(char)*SIZE_BUFF);
  while(1){
    int x=accept(p->sock,NULL,NULL);
    printf("accept %d \n",x);
    
    ssize_t res=recv(x,buffer,SIZE_BUFF,0);//Reception du nom
    if(-1==res){
      fprintf(stderr,"problème recv fileName : %s.\n",strerror(errno));
      free(buffer);
      close(x);
      exit(EXIT_FAILURE);
    }
    if(res==0){/*Pas d'octet reçu, Pas protocolaire, donc le main à clos la socket listen*/
      pthread_exit(NULL);
    }
  }
}


void IAMNOLONGERSERVER(int sock){
  if(close(sock)==-1)fprintf(stderr,"problème close sockListen : %s.\n",strerror(errno));
}
