server : server.o
	cc -o server server.o

server.o : server.c
	cc -c server.c

client : client.o
	cc -o client client.o

client.o : client.c
	cc -c client.c

.PHONY : clean
clean :
	-rm server client server.o client.o
