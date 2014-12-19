#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

void *thread_main(void *arg);
int add_all(int v1, ...);

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

  int sum;
  sum = add_all(1,2,3,4,5,6,7);

  return 0;
}

int add_all(int v1, ...) {
  va_list ap;

  int i, sum;
  sum = 0;

  va_start(ap, v1);
  for (i = v1; i != -1; i = va_arg(ap, int)) {
    sum += i;
  }
  va_end(ap);

  return sum;
}
