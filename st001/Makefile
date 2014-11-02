ST_ROOT = /home/bob/state-threads/FREEBSD_10.1-PRERELEASE_OPT/

stdemo : stdemo.o
	cc -L$(ST_ROOT) -lst -o stdemo stdemo.o

stdemo.o : stdemo.c
	cc -c -I$(ST_ROOT) stdemo.c

.PHONY : clean
clean :
	-rm stdemo stdemo.o
