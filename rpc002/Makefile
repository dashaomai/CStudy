ST_ROOT = ~/Libs/CLibs/state-threads/obj/

all : rpc_queue.o rpc_peer.o rpc_main.o
	cc -L $(ST_ROOT) -lst -o rpc rpc_queue.o rpc_peer.o rpc_main.o

rpc_queue.o : rpc_queue.h rpc_queue.c
	cc -g -c -I $(ST_ROOT) rpc_queue.c

rpc_peer.o : rpc_peer.h rpc_peer.c
	cc -g -c -I $(ST_ROOT) rpc_peer.c

rpc_main.o : rpc_main.c
	cc -g -c -I $(ST_ROOT) rpc_main.c

.PHONY : clean
clean :
	-rm -rf rpc *.o *.core
