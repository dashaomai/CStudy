ST_ROOT = ~/Libs/CLibs/state-threads/obj/
COMPILE=-g -c -I $(ST_ROOT) -Wall
LINKAGE=-L $(ST_ROOT) -lst

all : rpc_log.o rpc_queue.o rpc_peer.o rpc_main.o
	cc $(LINKAGE) -o rpc *.o

rpc_log.o : rpc_log.h rpc_log.c
	cc $(COMPILE) rpc_log.c

rpc_queue.o : rpc_queue.h rpc_queue.c
	cc $(COMPILE) rpc_queue.c

rpc_peer.o : rpc_peer.h rpc_peer.c
	cc $(COMPILE) rpc_peer.c

rpc_main.o : rpc_main.c
	cc $(COMPILE) rpc_main.c

.PHONY : clean
clean :
	-rm -rf rpc *.o *.core
