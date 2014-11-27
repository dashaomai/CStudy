ST_ROOT = ~/Libs/CLibs/state-threads/obj/

all : rpc.o
	cc -L $(ST_ROOT) -lst -o rpc rpc.o

rpc.o : rpc.c
	cc -c -I $(ST_ROOT) rpc.c

.PHONY : clean
clean :
	-rm rpc rpc.o
