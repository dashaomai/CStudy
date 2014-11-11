ST_ROOT = /Users/bob/Libs/CLibs/state-threads/DARWIN_14.0.0_OPT/

stdemo : stdemo.o
	cc -L$(ST_ROOT) -lst -o stdemo stdemo.o

stdemo.o : stdemo.c
	cc -c -I$(ST_ROOT) stdemo.c

.PHONY : clean
clean :
	-rm stdemo stdemo.o
