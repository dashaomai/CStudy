#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parameter.h"

int main(int argc, const char *argv[])
{
  struct method m;
  m.name = "rpc_call";
  m.length = strlen(m.name);

  struct parameter_queue *queue;
  queue = parameter_queue_alloc();

  int8_t    int8 = -15;
  uint8_t   uint8 = 196;
  int16_t   int16 = -4096;
  uint16_t  uint16 = 65531;

  parameter_queue_put(queue, parameter_alloc(UINT16, &(uint16)));
  parameter_queue_put(queue, parameter_alloc(INT16, &(int16)));
  parameter_queue_put(queue, parameter_alloc(INT8, &(int8)));
  parameter_queue_put(queue, parameter_alloc(UINT8, &(uint8)));
  parameter_queue_put(queue, parameter_alloc(UINT16, &(int8)));
  parameter_queue_put(queue, parameter_alloc(INT16, &(uint8)));

  assert(queue->count == 6);

  return 0;
}
