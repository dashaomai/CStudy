#include <stdint.h>

#ifndef PARAMETER_H
#define PARAMETER_H

enum parameter_type {
    INT8 = '\0', UINT8,
    INT16, UINT16,
    INT32, UINT32,
    INT64, UINT64,

    FLOAT32, UFLOAT32,
    FLOAT64, UFLOAT64,

    BOOLEAN,

    STRING,

    ARRAY
};

struct parameter {
    enum parameter_type   type;         // 当前参数的类型
    uint16_t              length;
    enum parameter_type   scala_type;   // 只有 type == ARRAY 时才有的标量类型
    uint16_t              scala_count;  // 只有 type == ARRAY 时才有的数组长度
    void                  *value;
    struct parameter      *next;
};

struct parameter_queue {
    uint8_t   count;
    struct parameter *head;
    struct parameter *tail;
};

struct parameter_queue *parameter_queue_alloc(void);
void parameter_queue_free(struct parameter_queue *params);

void parameter_queue_put(struct parameter_queue *params, struct parameter *param);
struct parameter *parameter_queue_get(struct parameter_queue *params);

struct parameter *parameter_alloc(const enum parameter_type type, const void *value);
struct parameter *parameter_alloc_array(const enum parameter_type type, const void *array, const uint16_t length);
void parameter_free(struct parameter *param);

#endif
