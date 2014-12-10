ST_ROOT = ~/Libs/CLibs/state-threads/obj/

all : stmain.o io.o
	cc -L $(ST_ROOT) -lst -o stmain stmain.o io.o

stmain.o : stmain.c
	cc -g -c -I $(ST_ROOT) stmain.c

io.o : io.c
	cc -g -c -I $(ST_ROOT) io.c

.PHONY : clean
clean :
	-rm -rf stmain *.o *.core
