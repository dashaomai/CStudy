#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "parameter.h"
#include "serial.h"

int sum_all(uint16_t v1, int16_t v2, int8_t v3, uint8_t v4, uint16_t v5, int16_t v6);
int sum_all_proxy(struct parameter_queue *queue);

float sum_float(float *floats, uint16_t count);
float sum_float_proxy(struct parameter *param);

int main(int argc, const char *argv[])
{
  struct parameter_queue *queue;
  queue = parameter_queue_alloc();

  int8_t    int8 = -15;
  uint8_t   uint8 = 196;
  int16_t   int16 = -4096;
  uint16_t  uint16 = 65531;

  float float16s[] = { 3.14, 0.618, 0., -15.5f };

  printf("Construction an array with %u float16_t(s)\n", sizeof(float16s) / sizeof(float16s[0]));

  parameter_queue_put(queue, parameter_alloc(UINT16, &(uint16)));
  parameter_queue_put(queue, parameter_alloc(INT16, &(int16)));
  parameter_queue_put(queue, parameter_alloc(INT8, &(int8)));
  parameter_queue_put(queue, parameter_alloc(UINT8, &(uint8)));
  parameter_queue_put(queue, parameter_alloc(UINT16, &(int8)));
  parameter_queue_put(queue, parameter_alloc(INT16, &(uint8)));

  parameter_queue_put(queue, parameter_alloc_array(FLOAT32, float16s, sizeof(float16s) / sizeof(float16s[0])));

  assert(queue->count == 7);

  // 测试下参数队列的编、解码
  struct serial_binary *binary;
  void *bytes;
  binary = serial_encode(queue);
  bytes = binary->bytes + 2;

  printf("编码参数队列，得到以下结果：\n<< ");
  uint16_t i;

  for (i=0; i<binary->length; i++) {
    printf("%02x ", *(uint8_t*)(bytes + i));
  }

  printf(">>\n");

  // 输出原参数队列的计算结果
  printf("The 1'st sum_all_proxy result is: %d\n", sum_all_proxy(queue));
  printf("The 1'st sum_float_proxy result is : %f\n", sum_float_proxy(parameter_queue_get(queue)));

  parameter_queue_free(queue);

  queue = serial_decode(binary);
  free(binary);

  // 输出解码参数队列的计算结果
  printf("The 2'nd sum_all_proxy result is: %d\n", sum_all_proxy(queue));
  printf("The 2'nd sum_float_proxy result is : %f\n", sum_float_proxy(parameter_queue_get(queue)));

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

  int sum = sum_all(*(uint16_t*)p1->value, *(int16_t*)p2->value, *(int8_t*)p3->value, *(uint8_t*)p4->value, *(uint16_t*)p5->value, *(int16_t*)p6->value);

  parameter_free(p1);
  parameter_free(p2);
  parameter_free(p3);
  parameter_free(p4);
  parameter_free(p5);
  parameter_free(p6);

  return sum;
}

int sum_all(uint16_t v1, int16_t v2, int8_t v3, uint8_t v4, uint16_t v5, int16_t v6) {
  return v1 + v2 + v3 + v4;
}

float sum_float_proxy(struct parameter *param) {
  float sum = sum_float((float*)param->value, param->scala_count);

  parameter_free(param);

  return sum;
}

float sum_float(float *floats, uint16_t count) {
  uint16_t i;
  float float16, sum;

  sum = 0.0;

  for (i = 0; i < count; i++) {
    float16 = floats[i];
    sum += float16;
  }

  return sum;
}
