#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parameter.h"

struct parameter_queue *parameter_queue_alloc(void) {
    struct parameter_queue *queue;

    queue = (struct parameter_queue *)calloc(1, sizeof(*queue));

    return queue;
}

void parameter_queue_free(struct parameter_queue *queue) {
    free(queue);
}

void parameter_queue_put(struct parameter_queue *queue, struct parameter *param) {
    if (queue->count == 0) {
        queue->head = param;
    } else {
        queue->tail->next = param;
    }

    queue->tail = param;
    queue->count ++;
}

struct parameter *parameter_queue_get(struct parameter_queue *queue) {
    struct parameter *param;

    if (queue->count > 0) {
        param = queue->head;
        queue->head = param->next;
        param->next = NULL;

        queue->count --;

        if (queue->count == 0) {
            assert(queue->head == NULL);
            queue->tail = NULL;
        }
    } else {
        param = NULL;
    }

    return param;
}

struct parameter *parameter_alloc(const enum parameter_type type, const void *value) {
    struct parameter *param;
    uint16_t length;

    switch (type) {
        case INT8:
        case UINT8:
        case FLOAT8:
        case UFLOAT8:
        case BOOLEAN:
            length = 1;
            break;

        case INT16:
        case UINT16:
        case FLOAT16:
        case UFLOAT16:
            length = 2;
            break;

        case INT32:
        case UINT32:
        case FLOAT32:
        case UFLOAT32:
            length = 4;
            break;

        case STRING:
            length = strlen((char const *)value);
            break;

        case ARRAY:
            // TODO: 还没想好怎么面对 ARRAY 参数类型
            return NULL;
    }

    // 在连续内存空间上分配结构体内容
    param = (struct parameter *)calloc(1, sizeof(*param) + length);
    param->type = type;
    param->value = (void*)param + sizeof(*param);      // 让变长值内容存放于 param 紧随其后的内存地址中
    param->length = length;

    memcpy(param->value, value, length);

    return param;
}

struct parameter *parameter_alloc_array(const enum parameter_type type, const void *array, const uint16_t length) {
  struct parameter *param;
  uint16_t unit_length;
  uint16_t total_length;

  switch (type) {
    case INT8:
    case UINT8:
    case FLOAT8:
    case UFLOAT8:
    case BOOLEAN:
      unit_length = 1;
      total_length = length * unit_length;
      break;

    case INT16:
    case UINT16:
    case FLOAT16:
    case UFLOAT16:
      unit_length = 2;
      total_length = length * unit_length;
      break;

    case INT32:
    case UINT32:
    case FLOAT32:
    case UFLOAT32:
      unit_length = 4;
      total_length = length * unit_length;
      break;

    case STRING:
      // TODO: 暂不支持 ARRAY 下挂 STRING
      return NULL;

    case ARRAY:
      // TODO: 暂不支持 ARRAY 下挂 ARRAY
      return NULL;
  }

  // 在连续内存空间上分配结构体内容
  param = (struct parameter *)calloc(1, sizeof(*param) + total_length);
  param->type = ARRAY;
  param->scala_type = type;
  param->scala_count = length;
  param->value = (void*)param + sizeof(*param);          // 让值内容存放于 param 紧随其后的内存地址当中
  param->length = total_length;

  memcpy(param->value, array, total_length);

  return param;
}

void parameter_free(struct parameter *param) {
  // free(param->value);
  free(param);
}
