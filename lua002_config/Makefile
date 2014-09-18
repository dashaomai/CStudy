luaconfig : luaconfig.o
	cc -o luaconfig luaconfig.o -L/usr/local/lib -llua

luaconfig.o : ../lua/common.h
	cc -c luaconfig.c -I/usr/local/include

test : luaconfig config.lua
	./luaconfig config.lua

.PHONY : clean
clean :
	-rm luaconfig luaconfig.o
