ST_ROOT = ~/Libs/CLibs/state-threads/obj/
CC = cc
COMPILE = -g -c -I $(ST_ROOT)
LINK =

all : stmain.o io.o
	$(CC) $(LINK) -o stmain stmain.o io.o $(ST_ROOT)libst.a

stmain.o : stmain.c
	$(CC) $(COMPILE) stmain.c

io.o : io.c
	$(CC) $(COMPILE) io.c

.PHONY : clean
clean :
	-rm -rf stmain *.o *.core
