ST_ROOT = ~/Libs/CLibs/state-threads/obj/

all : stmain.o
	cc -L $(ST_ROOT) -lst -o stmain stmain.o

stmain.o : stmain.c
	cc -g -c -I $(ST_ROOT) stmain.c

.PHONY : clean
clean :
	-rm -rf stmain *.o *.core
