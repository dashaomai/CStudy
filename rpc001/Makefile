ST_ROOT = ~/Libs/CLibs/state-threads/obj/

all : rpc.o
	cc -L $(ST_ROOT) -lst -o rpc rpc.o

rpc.o : rpc.c
	cc -g -std=gnu99 -c -I $(ST_ROOT) rpc.c

.PHONY : clean
clean :
	-rm -rf rpc *.o *.core
