all : crash.o
	cc -o crash crash.o

crash.o : crash.c
	cc -g -c crash.c 

.PHONY : clean
clean :
	-rm -rf crash *.o *.core
