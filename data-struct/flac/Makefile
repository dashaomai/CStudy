CC = cc
COMPILE = -Wall -g -c -std=c99
LINK =
EXE = flac

all : main.o
	$(CC) $(LINK) -o $(EXE) *.o

main.o : main.c
	$(CC) $(COMPILE) main.c

.PHONY : clean
clean :
	-rm -rf $(EXE) *.o *.core
