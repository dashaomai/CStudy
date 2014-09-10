luaer : luaer.o
	cc -o luaer luaer.o -L/usr/local/lib -llua

luaer.o :
	cc -c luaer.c -I/usr/local/include

test : luaer hello.lua
	./luaer hello.lua

.PHONY : clean
clean :
	-rm luaer luaer.o
