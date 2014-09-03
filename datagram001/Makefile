all : listener

listener : listener.o
	cc -o listener listener.o

listener.o : listener.c
	cc -c listener.c

.PHONY : clean
clean :
	-rm listener listener.o
