#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define LEN 5
#define COUNT 5

int main(int argc, const char *argv[])
{
  uint16_t *arr;
  int size = sizeof(uint16_t) * LEN;
  uint16_t *arr1, *arr2, *arr3, *arr4, *arr5;
  uint16_t v;

  arr = (uint16_t*)malloc(size * COUNT);

  arr1 = arr;

  arr2 = arr1 + size;
  arr3 = arr2 + size;
  arr4 = arr3 + size;
  arr5 = arr4 + size;

  srand(time(0));

  for (int i = 1; i<=LEN; i++) {
    v = rand();

    arr1[i] = v;
    arr2[i] = v;
    arr3[i] = v;
    arr4[i] = v;
    arr5[i] = v;

    printf("\t%u", v);

    if (i % 10 == 0) {
      printf("\n");
    }
  }
  printf("\n");

  free(arr); arr = NULL;
  arr1 = arr2 = arr3 = arr4 = arr5 = NULL;

  return 0;
}
