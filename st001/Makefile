ST_ROOT = ~/Libs/CLibs/state-threads/obj/

stdemo : stdemo.o
	cc -L $(ST_ROOT) -lst -o stdemo stdemo.o

stdemo.o : stdemo.c
	cc -g -c -I $(ST_ROOT) stdemo.c

.PHONY : clean
clean :
	-rm -rf stdemo stdemo.o
