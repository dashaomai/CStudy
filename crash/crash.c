#include <stdio.h>

void crash(void) {
  fprintf(stdout, "hello crash!\n");

  char overflow[0xFFFFFF];
}

int main(int argc, char **argv) {
  crash();

  return 0;
}
