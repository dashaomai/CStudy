stdemo : stdemo.o
	cc -o stdemo stdemo.o -L/Users/bob/Libs/CLibs/state-threads/DARWIN_14.0.0_OPT/ -lst

stdemo.o : stdemo.c
	cc -c -I/Users/bob/Libs/CLibs/state-threads/DARWIN_14.0.0_OPT/ stdemo.c

.PHONY : clean
clean :
	-rm stdemo stdemo.o
