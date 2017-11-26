/*classique*/
#include <unistd.h>/*chdir*/
#include <stdio.h>
#include <stdlib.h>

/*fonctions reseau*/
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
