#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main(int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: luaer <Path to the lua file>\n");
    return 1;
  }

  lua_State *L = luaL_newstate();

  luaL_openlibs(L);

  luaL_dofile(L, argv[1]);

  lua_close(L);

  return 0;
}
