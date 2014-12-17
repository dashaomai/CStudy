CC = cc
COMPILE = -g -c
LINK =

all : serial.o parameter.o
	$(CC) $(LINK) -o serial *.o

serial.o : serial.c
	$(CC) $(COMPILE) serial.c

parameter.o : parameter.c parameter.h
	$(CC) $(COMPILE) parameter.c

.PHONY : clean
clean :
	-rm -rf serial *.o *.core
