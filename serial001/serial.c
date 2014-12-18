#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parameter.h"

int sum_all(uint16_t v1, int16_t v2, int8_t v3, uint8_t v4, uint16_t v5, int16_t v6);
int sum_all_proxy(struct parameter_queue *queue);

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

  printf("The sum_all_proxy result is: %d\n", sum_all_proxy(queue));

  return 0;
}

int sum_all_proxy(struct parameter_queue *queue) {
  struct parameter *p1, *p2, *p3, *p4, *p5, *p6;

  p1 = parameter_queue_get(queue);
  p2 = parameter_queue_get(queue);
  p3 = parameter_queue_get(queue);
  p4 = parameter_queue_get(queue);
  p5 = parameter_queue_get(queue);
  p6 = parameter_queue_get(queue);

  return sum_all(*(uint16_t*)p1->value, *(int16_t*)p2->value, *(int8_t*)p3->value, *(uint8_t*)p4->value, *(uint16_t*)p5->value, *(int16_t*)p6->value);
}

int sum_all(uint16_t v1, int16_t v2, int8_t v3, uint8_t v4, uint16_t v5, int16_t v6) {
  return v1 + v2 + v3 + v4;
}
