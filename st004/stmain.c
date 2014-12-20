#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

static void *thread_main(void *arg);
static int add_all(int v1, ...);

int main(int argc, const char *argv[])
{
  st_set_eventsys(ST_EVENTSYS_ALT);

  st_init();

  printf("entering main.\n");

  st_thread_create(thread_main, NULL, 0, 0);

  int sum;
  sum = add_all(8, 9, 10, 11, 12);

  printf("add_all(8, 9, 10, 11, 12) = %d\n", sum);

  st_sleep(1);

  return 0;
}

void *thread_main(void *arg) {
  // this line should cause Error on FreeBSD 10.1 (X64 or Amd64)
  printf("entering the state-threads #%d.\n", 2);

  int sum;
  sum = add_all(1,2,3,4,5,6,7);

  return 0;
}

int add_all(int v1, ...) {
  va_list ap;

  int i, j, sum;
  sum = 0;

  va_start(ap, v1);
  for (i = v1, j = 0; j <= 4; i = va_arg(ap, int), j++) {
    printf("i = %d, sum = %d\n", i, sum);
    sum += i;
  }
  va_end(ap);

  return sum;
}
