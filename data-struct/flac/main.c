#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int flac(int a) {
  if (a == 0) return 0;
  else if (a == 1) return 1;

  return a * flac(a - 1);
}

int flac_tail(int a, int v) {
  if (a == 0) return 0;
  else if (a == 1) return v;

  return flac_tail(a - 1, a * v);
}

int main(int argc, char **argv) {
  printf("5! = %d\n", flac(5));
  printf("5! = %d\n", flac_tail(5, 1));

  return EXIT_SUCCESS;
}
