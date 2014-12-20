ST_ROOT = ~/Libs/CLibs/state-threads/obj/
CC = cc
COMPILE = -O3 -c -I $(ST_ROOT)
LINK =

all : stmain.o
	$(CC) $(LINK) -o stmain *.o $(ST_ROOT)libst.a

stmain.o : stmain.c
	$(CC) $(COMPILE) stmain.c

.PHONY : clean
clean :
	-rm -rf stmain *.o *.core
