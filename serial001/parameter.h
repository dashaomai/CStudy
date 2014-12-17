#include <stdint.h>

#ifndef PARAMETER_H
#define PARAMETER_H

struct method {
    uint8_t   length;
    char      *name;
};

enum parameter_type {
    INT8 = '\0', UINT8,
    INT16, UINT16,
    INT32, UINT32,
    INT64, UINT64,

    FLOAT8, UFLOAT8,
    FLOAT16, UFLOAT16,
    FLOAT32, UFLOAT32,
    FLOAT64, UFLOAT64,

    BOOLEAN,

    STRING,

    ARRAY
};

struct parameter {
    enum parameter_type   type;
    void      *value;
    uint16_t  length;
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
void parameter_free(struct parameter *param);

#endif