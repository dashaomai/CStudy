ST_ROOT = /home/bob/state-threads/FREEBSD_10.1-PRERELEASE_OPT/

stdemo : stdemo.o
	cc -o stdemo stdemo.o -L$(ST_ROOT) -lst

stdemo.o : stdemo.c
	cc -c -I$(ST_ROOT) stdemo.c

.PHONY : clean
clean :
	-rm stdemo stdemo.o
