#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define LEN 100

int main(int argc, const char *argv[])
{
  uint16_t *arr;

  arr = (uint16_t*)malloc(sizeof(uint16_t) * LEN);
  srand(time(0));

  for (int i = 1; i<=LEN; i++) {
    arr[i] = rand();
    printf("\t%u", arr[i]);

    if (i % 10 == 0) {
      printf("\n");
    }
  }
  printf("\n");

  free(arr), arr = NULL;

  return 0;
}
