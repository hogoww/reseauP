FLAGS = -Wall -ansi -pedantic -std=c99
POSTFLAGS= -lpthread
all:
	gcc $(FLAGS) -c	listAssoc.c $(POSTFLAGS)
	#gcc $(FLAGS) -o s server.c listAssoc.o
	gcc $(FLAGS) -o p pair.c listAssoc.o $(POSTFLAGS)
