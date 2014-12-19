ST_ROOT = ~/Libs/CLibs/state-threads/obj/
CC=cc
COMPILE=-g -c -I $(ST_ROOT) -Wall
LINKAGE=

all : rpc_log.o rpc_queue.o rpc_parameter.o rpc_serial.o rpc_protocol.o rpc_peer.o rpc_main.o
	$(CC) $(LINKAGe) -o rpc *.o $(ST_ROOT)libst.a

rpc_log.o : rpc_log.h rpc_log.c
	$(CC) $(COMPILE) rpc_log.c

rpc_queue.o : rpc_queue.h rpc_queue.c
	$(CC) $(COMPILE) rpc_queue.c

rpc_parameter.o : rpc_parameter.h rpc_parameter.c
	$(CC) $(COMPILE) rpc_parameter.c

rpc_serial.o : rpc_serial.h rpc_serial.c
	$(CC) $(COMPILE) rpc_serial.c

rpc_protocol.o : rpc_protocol.h rpc_protocol.c
	$(CC) $(COMPILE) rpc_protocol.c

rpc_peer.o : rpc_peer.h rpc_peer.c
	$(CC) $(COMPILE) rpc_peer.c

rpc_main.o : rpc_main.c
	$(CC) $(COMPILE) rpc_main.c

.PHONY : clean
clean :
	-rm -rf rpc *.o *.core
