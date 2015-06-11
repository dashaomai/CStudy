CC = cc
COMPILE = -g -c -std=c99
LINK =
EXE = sorts

all : main.o
	$(CC) $(LINK) -o $(EXE) *.o

main.o : main.c
	$(CC) $(COMPILE) main.c

.PHONY : clean
clean :
	-rm -rf $(EXE) *.o *.core
