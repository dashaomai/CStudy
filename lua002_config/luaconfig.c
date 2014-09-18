#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../lua/common.h"

int main(int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: luaconfig <Path To the lua config>\n");
    return 1;
  }

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  if (luaL_loadfile(L, argv[1]) || lua_pcall(L, 0, 0, 0))
    error(L, "cannot run config. file: %s", lua_tostring(L, -1));

  lua_getglobal(L, "width");
  lua_getglobal(L, "height");

  if (!lua_isnumber(L, -2))
    error(L, "'width' should be a number\n");

  if (!lua_isnumber(L, -1))
    error(L, "'height' should be a number\n");

  lua_Integer w = lua_tointeger(L, -2);
  lua_Integer h = lua_tointeger(L, -1);

  lua_close(L);

  printf("width = %ld, height = %ld", w, h);

  return 0;
}
