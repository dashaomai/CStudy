#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

void *thread_main(void *arg);

int main(int argc, const char *argv[])
{
  st_init();

  printf("[%d] 当前在主线程内\n", 1);

  st_thread_create(thread_main, NULL, 0, 0);

  st_sleep(1);

  return 0;
}

void *thread_main(void *arg) {
  printf("[%d] 当前在 state-threads 线程内\n", 2);

  return 0;
}
