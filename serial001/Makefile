CC = cc
COMPILE = -g -c
LINK =

all : main.o parameter.o
	$(CC) $(LINK) -o serial *.o

main.o : main.c
	$(CC) $(COMPILE) main.c

parameter.o : parameter.c parameter.h
	$(CC) $(COMPILE) parameter.c

.PHONY : clean
clean :
	-rm -rf serial *.o *.core
