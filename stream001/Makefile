objects = server.o

server : server.o
	cc -o server $(objects)

server.o : server.c
	cc -c server.c

.PHONY : clean
clean :
	-rm server $(objects)
