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

/*IPC*/
#include <sys/ipc.h>
#include <sys/msg.h>

/*erreur*/
#include <string.h>
#include <errno.h>

/*Manipulation dossier/fichiers*/
#include <dirent.h>
#include <sys/stat.h>

/*Threading*/
#include <pthread.h>

/*API personnelles*/
#include "listAssoc.h"
#include "shared_define.h"



struct servParam{
  int sock;
};

struct dllFile{
  char* adresse;
  uint16_t port;
  char *file;
};

struct sendpart{
  int sock;
  int msgid;
};

struct msgend{
  long label;
  pthread_t pid;
};

/*Connection*/
int ConnectToServ(char* servAddress,uint16_t port);
void DisconnectFromServ(int sock);

/*interaction Annuaire - pair*/
void DisAuServeurQueJeSuisPresent(char* servAddress,uint16_t port);
void DisAuServeurQueJeQuitte(char* servAddress,uint16_t port);
void QueryTypeServ(int serv,char type);
struct listAssoc* RefreshThatList(char* servAddress,uint16_t port);

/*peer to peer side*/
int createServerSocket(uint16_t port);
void* IAMSERVEURNOW(void* param);
void IAMNOLONGERSERVER();
void ConnectToThatPeer(struct listAssoc* peer,uint16_t port);
void* getFileFromThatPeer(void* param);
void* sendfile(void* param);

/*Manipulation*/
int LookForEnd(char s[],int size);
char* ConvertBracketToStar(char s[],int fin,int size);
int CharPToInt(char* c,int size);
int getIntFromServ(int serv);


int main(int argc,char** argv){
  if(argc!=3){/*Check arguments*/
    fprintf(stderr,"\nUsage : ./p [adresse_annuaire] [Port_Annuaire] \n\n");
    exit(EXIT_FAILURE);
  }

  char* servAddress=argv[1];
  uint16_t port=htons(atoi(argv[2]));

  DisAuServeurQueJeSuisPresent(servAddress,port);
  
  pthread_t server;/*Lancement serveur*/
  struct servParam *sp=malloc(sizeof(struct servParam));
  sp->sock=createServerSocket(port+1);/*Pour ne pas etre sur le meme port que le serveur annuaire*/
  if(-1==pthread_create(&server,NULL,IAMSERVEURNOW,(void*)sp)){
    fprintf(stderr,"problème creation thread serveur: %s.\n",strerror(errno));
  }
  

  int check=chdir("./reception");//On se met dans le bon dossier de reception
  if(check==-1){
    fprintf(stderr,"problème chdir into reception : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }

  
  struct listAssoc* list=NULL;
  list=RefreshThatList(servAddress,port);
  DisplayListAssoc(list);
  int done=0;
  int numPeer;
  struct listAssoc* peer=NULL;
  //struct list* file;
  do{
    printf("\nC -> Connecte à un pair\nR -> Refresh la liste de l'annuaire\nQ-> Quitte le reseau\n");
    
    switch(fgetc(stdin)){
      fgetc(stdin);//remove the input char
    case 'r':
    case 'R':
      printf("Refresh de la liste des pairs\n");
      delete_listAssoc_and_key_and_values(list);
      list=RefreshThatList(servAddress,port);
      DisplayListAssoc(list);
      break;
    case 'q':
    case 'Q':
      done=1;
      break;
    case 'c':
    case 'C':
      printf("Entrez le numéro du pair correspondant à la liste.\n");
      DisplayListAssoc(list);
      scanf("%d",&numPeer);
      if((peer=getIndex_listAssoc(list,numPeer))){
	printf("Connection au pair %d = %s.\nQuel est le numero du fichier que vous souhaitez télécharger?\n",numPeer,peer->k);
	int numfile;
	scanf("%d",&numfile);
	struct list* filename;
	if((filename=getIndex_list(peer->l,numfile))){
	  
	  struct dllFile* p=malloc(sizeof(struct dllFile*));
	  p->adresse=peer->k;
	  p->port=port+1;
	  p->file=filename->v;
	  getFileFromThatPeer((void*)p);/*pour ne pas etre au meme niveau que l'accés à l'annuaire*/
	}
	else{
	  printf("\n Numéro de fichier invalide.\n");
	}
      }
      else{
	printf("Ce chiffre ne correspond pas à un pair.\n");
      }
      break;
      fgetc(stdin);
      
    default:
      printf("Commande non reconnue.\n");
      break;
    }
      
  }while(!done);

  delete_listAssoc_and_key_and_values(list);
  //free(list);
  IAMNOLONGERSERVER(sp->sock);
  printf("signal d'arret envoyé au serveur\n");
  DisAuServeurQueJeQuitte(servAddress,port);
  printf("J'ai dis au serveur annuaire que je partais\n");

  printf("Et j'attends que le serveur soit terminé (il attends que les envois soit finis)\n");
  //pthread_join(server,NULL);
  printf("serveur eteint, bonne journée:)\n");

  return 0;
}

int ConnectToServ(char* servAddress,uint16_t port){
  int sock=socket(PF_INET,SOCK_STREAM,0);//création de la socket
  if(sock==-1){fprintf(stderr,"Probleme ConnectToserv socket() : %s.\n",strerror(errno));exit(EXIT_FAILURE);}
  
  struct sockaddr_in addr;//et remplissage des infos passé en ligne de commande.
  memset(&addr,0,sizeof(struct sockaddr));
  addr.sin_family=AF_INET;

  int checkReturn=inet_pton(AF_INET,servAddress,&(addr.sin_addr));
  if (checkReturn==0 || checkReturn==-1){
    fprintf(stderr, "ConnectToServ : Adresse fournie format incorrect ; %s.\n",strerror(errno));
    DisconnectFromServ(sock);
    exit(EXIT_FAILURE);
  }
  addr.sin_port=port;

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
  char* res=malloc(sizeof(char)*end);
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

void DisAuServeurQueJeSuisPresent(char* servAddress,uint16_t port){
  int serv=ConnectToServ(servAddress,port);

  QueryTypeServ(serv,COMING);
  char *buffer=malloc(sizeof(char)*SIZE_BUFF);
  DIR* d;
  struct dirent* dir;
  d=opendir("./Seed");
  if(d){
    while ((dir = readdir(d)) != NULL){
      if((dir->d_name[0]=='.')){
	continue;/*Pour ne pas afficher les fichier cachés*/
      }

      int end=LookForEnd(dir->d_name,256);
      char* r=ConvertBracketToStar(dir->d_name,end,256);
      strcpy(buffer,r);
      printf("sending %s\n",buffer);
      ssize_t res=send(serv,buffer,SIZE_BUFF,0);//Envois du nom du fichier au serveur.
      
      if(-1==res){
	fprintf(stderr,"DisAuServeurQueJeSuisPresent() problème envoie d'un nom de fichier : %s.\n",strerror(errno));
	closedir(d);
	DisconnectFromServ(port);
	exit(EXIT_FAILURE);
      }
      free(r);
    }

    buffer[0]='\0';
    ssize_t res=send(serv,buffer,0,0);//fin des fichiers à réceptionner.
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

  free(buffer);
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


void DisAuServeurQueJeQuitte(char* servAddress,uint16_t port){
  int serv=ConnectToServ(servAddress,port);
  
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


struct listAssoc* RefreshThatList(char* servAddress,uint16_t port){/*Not the most optimize, but that'll be enough*/
  int serv=ConnectToServ(servAddress,port);
  QueryTypeServ(serv,REFRESH);
  
  
  char* buffer=malloc(sizeof(char)*SIZE_BUFF);

  struct listAssoc* list=NULL;
  ssize_t res1,res2;
  
  do{
    res1=recv(serv,buffer,SIZE_BUFF,0);//Reception du nom du ième pair
    if(-1==res1){
      fprintf(stderr,"problème recv pair name, RefreshThatList : %s.\n",strerror(errno));
      free(buffer);
      DisconnectFromServ(serv);
      exit(EXIT_FAILURE);
    }
    
    if(res1==0 || buffer[0]=='\0'){
      break;
    }

    char* ip=resize(buffer,SIZE_BUFF);
    printf("ip=%s\n",ip);

    if(!list){
      list=make_ListAssoc(ip);
    }
    
    struct list* lt=NULL;
   
    do{
      res2=recv(serv,buffer,SIZE_BUFF,0);//Reception du jeme filename du ieme pair
      if(-1==res2){
	fprintf(stderr,"problème recv fileName RefreshThatList : %s.\n",strerror(errno));
	free(buffer);
	DisconnectFromServ(serv);
	exit(EXIT_FAILURE);
      }
      
      if(res2==0 || buffer[0]=='\0'){
	break;
      }
      
      printf("recv filename %s\n",buffer);
      char* fileName=resize(buffer,SIZE_BUFF);
      lt=add_value_list(lt,fileName);
    }while(1);
    
    destroyAndChangeList_listAssoc(list,ip,lt);

  }while(1);

  free(buffer);
  
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

void* getFileFromThatPeer(void* param){

  struct dllFile* p=(struct dllFile*)param;
  int serv=ConnectToServ(p->adresse,p->port);

  int len=0;
  while(p->file!='\0'){len++;}//recherche taille du nom du fichier.
  
  char* buffer=malloc(SIZE_BUFF*sizeof(char));
  int *done=malloc(sizeof(int));

  ssize_t res=send(serv,p->file,len+1,0);//Envois du nom du fichier au serveur.
  if(-1==res){
    fprintf(stderr,"problème sendName : %s.\n",strerror(errno));
    DisconnectFromServ(serv);
    free(buffer);
    exit(EXIT_FAILURE);
  }
 
  int* s=malloc(sizeof(int));
  res=recv(serv,s,sizeof(int),0);//Reception d'un certain nombre d'octet.
  if(-1==res){
    fprintf(stderr,"problème recv size file : %s.\n",strerror(errno));
    free(buffer);
    free(s);
    DisconnectFromServ(serv);
    exit(EXIT_FAILURE);
  }
  int size=*s;
  free(s);
  
  if(size!=0){//File doesn't exist    
  
    FILE* fileToReceive=fopen(p->file,"w");//on ouvre en écriture le fichier que l'on va télécharger de ce serveur.
    if(fileToReceive==NULL){
      fprintf(stderr,"file failed to open %s : %s.\n",p->file,strerror(errno));
      DisconnectFromServ(serv);
      free(buffer);
      exit(EXIT_FAILURE);
    }



    int som=0;//Nombre d'octet reçu.
    while(1){//Boucle infinie, la condition de fin est géré dedans.
    
      ssize_t res=recv(serv,buffer,SIZE_BUFF,0);//Reception d'un certain nombre d'octet.
      if(-1==res){
	fprintf(stderr,"problème recv : %s.\n",strerror(errno));
	free(buffer);
	DisconnectFromServ(serv);
	free(done);
	exit(EXIT_FAILURE);
      }


      if(res!=0){//Et on l'écrit dans le fichier.
	som+=res;//On ajoute le nombre d'octet reçu par le dernier envois du serveur.     
	ssize_t r=fwrite(buffer,1,res,fileToReceive);
	if(res!=r){//en verifiant qu'il n'y ai pas de problème du write
	  printf("Pas inscrit le meme nombre de characteres que ce que j'ai reçu\n");
	}
      }

      res=recv(serv,done,sizeof(int),0);//Reception d'un certain nombre d'octet.
      if(-1==res){
	fprintf(stderr,"problème recv done : %s.\n",strerror(errno));
	free(buffer);
	free(done);
	DisconnectFromServ(serv);
	exit(EXIT_FAILURE);
      }
    

      if(*done)
	break;
    }

    if(som==size){//Output du nombre d'octect reçu
      printf("%d octets received, fichier correctement reçu.\n",som);
    }
    else{
      printf("Fichier de taille differente qu'annoncée, deletion de celui ci.\n");
      remove(p->file);
    }
  }
  
  free(buffer);
  free(done);

  DisconnectFromServ(serv);  
  return NULL;
}


void getThreads(int msgid,int* compteur,int waitflag){
  struct msgend buff;
  if(*compteur==0){
    return;
  }

  while(*compteur>=0){
    if(msgrcv(msgid,&buff,sizeof(struct msgend),0,waitflag)==-1){
      if(errno==ENOMSG){
	break;/*No other threads are done*/
      }
      else{
	fprintf(stderr,"probleme récupération des threads finis : %s.\n",strerror(errno));
	break;
      }
    }
    pthread_join(buff.pid,NULL);
    *compteur-=1;
  }
}

void* IAMSERVEURNOW(void* param){
  struct servParam* p=(struct servParam*)param;

  int k=ftok("./Seed",12);
  if(k==-1){fprintf(stderr,"ftok failed : %s.\n",strerror(errno));};
  int id=msgget(k, IPC_CREAT | 0666);
  

  int check=chdir("./Seed");//On se met dans le bon dossier de reception
  if(check==-1){
    fprintf(stderr,"problème chdir into seed : %s.\n",strerror(errno));
    exit(EXIT_FAILURE);
  }

  int compteur=0;
  while(1){
    int x=accept(p->sock,NULL,NULL);
    getThreads(id,&compteur,IPC_NOWAIT);
    if(x==-1){
      fprintf(stderr,"Accept failed : %d - %s.\n",errno,strerror(errno));
      break;
    }
    
    if(x==0){
      printf("server side done working.\n");
    }
    printf("accept %d \n",x); 
    
    struct sendpart *s=malloc(sizeof(struct sendpart));
    s->sock=x;
    s->msgid=id;
    pthread_t t;/*don't need it*/
    pthread_create(&t,NULL,sendfile,(void*)s);
    compteur++;
  }
  getThreads(id,&compteur,0);
  
  if(-1==msgctl(id,IPC_RMID,NULL)){fprintf(stderr,"Probleme en essayant de détruire la file de message : %s",strerror(errno));}
  
  pthread_exit(NULL);
}


void IAMNOLONGERSERVER(int sock){
  if(close(sock)==-1)fprintf(stderr,"problème close sockListen : %s.\n",strerror(errno));
}


void* sendfile(void* param){
  struct sendpart* p=(struct sendpart*)param;
  free(param);
  int descClient=p->sock;

  char *buffer=malloc(sizeof(char)*SIZE_BUFF);//Préparation du buffer d'envois
  
  ssize_t res=recv(descClient,buffer,SIZE_BUFF,0);//Reception du nom
  if(-1==res){
    fprintf(stderr,"problème recv filename : %s.\n",strerror(errno));
    free(buffer);
    close(descClient);
    exit(EXIT_FAILURE);
  }

  int j=0;
  while(buffer[j]!='\0'){//calcul de la longueur du nom
    j++;
  }
    
  char* filename=malloc(sizeof(char)*j+1);
    
  ssize_t i=0;
  while(buffer[i]!='\0'){
    filename[i]=buffer[i];
    i++;
  }
  filename[j]='\0';

  int* s=malloc(sizeof(int));
  struct stat st;
  if(0==stat(filename, &st)){//File doesn't exist
    printf("Fichier demandé : %s n'existe pas\n",filename);

    *s=0;
    res=send(descClient,s,sizeof(int),0);//signal de fichier_doesn't exist
    if(-1==res){
      fprintf(stderr,"problème send fichier %s n'existe pas : %s.\n",filename,strerror(errno));
      close(descClient);
      free(buffer);
      free(s);
      s=NULL;
      buffer=NULL;
      return NULL;
    }
    else{
      free(s);
    }
  }
  else{//File exist, on l'envoie
    printf("Envois du fichier %s\n",filename);
    *s=(int)st.st_size;
    res=send(descClient,s,sizeof(int),0);//Envois taille du fichier
    if(-1==res){
      fprintf(stderr,"problème send fichier %s n'existe pas : %s.\n",filename,strerror(errno));
      close(descClient);
      free(buffer);
      free(s);
      return NULL;
    }
    
      
    FILE* fileToSend=fopen(filename,"r");//Ouverture du fichier.
    if(fileToSend==NULL){//NOT SUPPOSED TO HAPPEND.
      fprintf(stderr,"file failed to open even though it was supposed to exist : %s.\n",strerror(errno));
      close(descClient);
      free(buffer);
      remove(filename);//On retire le fichier vide nouvellement créé.
    }

    int* done=malloc(sizeof(int));
    int c;
    while(*done==0){//sinon il envoit le fichier.
      i=0;
      while(i<SIZE_BUFF){//On remplis le buffer.
	c=fgetc(fileToSend);
	if(c==EOF){
	  *done=1;
	  break;
	}
	buffer[i]=c;
	i++;/**************PAS SUPER SUR !****************/
      }
      
      
      res=send(descClient,buffer,i,0);//Et on l'envois, jusqu'à la taille maximale, ou la fin du fichier
      if(-1==res){
	fprintf(stderr,"problème send : %s.\n",strerror(errno));
	close(descClient);
	free(buffer);
	free(filename);
	free(done);
	fclose(fileToSend);
	exit(EXIT_FAILURE);
      }
      
      
      res=send(descClient,done,sizeof(int),0);//à chaque tour de boucle on dis si on a finis
      if(-1==res){
	fprintf(stderr,"problème send : %s.\n",strerror(errno));
	close(descClient);
	free(buffer);
	free(done);
	free(filename);
	fclose(fileToSend);
	exit(EXIT_FAILURE);
      }
    }

    fclose(fileToSend);
    printf("done sending a file.\n");
    free(done);
    close(descClient);
  }

  free(buffer);
  free(filename);
  
  
  struct msgend mess;
  mess.label=1;/*doesn't matter*/
  mess.pid=pthread_self();
  msgsnd(p->msgid,(void*)&mess,sizeof(mess),0);

  return NULL;
}
