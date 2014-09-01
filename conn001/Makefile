objects = conn.o

conn : conn.o
	cc -o conn $(objects)

conn.o : conn.c
	cc -c conn.c

.PHONY : clean
clean :
	-rm conn $(objects)
