CC = cc
COMPILE = -g -c
LINK =

all : main.o parameter.o serial.o
	$(CC) $(LINK) -o serial *.o

main.o : main.c
	$(CC) $(COMPILE) main.c

parameter.o : parameter.c parameter.h
	$(CC) $(COMPILE) parameter.c

serial.o : serial.c serial.h
	$(CC) $(COMPILE) serial.c

.PHONY : clean
clean :
	-rm -rf serial *.o *.core
