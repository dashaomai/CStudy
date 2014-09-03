all : listener talker

listener : listener.o
	cc -o listener listener.o

listener.o : listener.c
	cc -c listener.c

talker : talker.o
	cc -o talker talker.o

talker.o : talker.c
	cc -c talker.c

.PHONY : clean
clean :
	-rm listener listener.o talker talker.o
