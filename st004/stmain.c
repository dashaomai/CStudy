#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

void *thread_main(void *arg);

int main(int argc, const char *argv[])
{
  st_init();

  printf("entering main.\n");

  st_thread_create(thread_main, NULL, 0, 0);

  st_sleep(1);

  return 0;
}

void *thread_main(void *arg) {
  // this line should cause Error on FreeBSD 10.1 (X64 or Amd64)
  printf("entering the state-threads.\n");

  return 0;
}
