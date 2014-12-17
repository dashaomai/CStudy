#include <stdlib.h>
#include <string.h>

#include "parameter.h"

struct parameter_queue *parameter_queue_alloc(void) {
    struct parameter_queue *params;

    params = (struct parameter_queue *)calloc(1, sizeof(*params));

    return params;
}

void parameter_queue_free(struct parameter_queue *params) {
    free(params);
}

void parameter_queue_put(struct parameter_queue *params, struct parameter *param) {
    if (params->count == 0) {
        params->head = param;
    } else {
        params->tail->next = param;
    }
    
    params->tail = param;
    params->count ++;
}

struct parameter *parameter_queue_get(struct parameter_queue *params) {
    struct parameter *param;
    
    if (params->count > 0) {
        param = params->head;
        params->head = param->next;
        param->next = NULL;

        params->count --;
        
        if (params->count == 0) {
            params->tail = NULL;
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

        case INT64:
        case UINT64:
        case FLOAT64:
        case UFLOAT64:
            length = 8;
            break;

        case STRING:
            length = strlen((char const *)value);
            break;

        case ARRAY:
            // TODO: 还没想好怎么面对 ARRAY 参数类型
            break;
    }

    // 在连续内存空间上分配结构体内容
    param = (struct parameter *)calloc(1, sizeof(*param) + length);
    param->type = type;
    param->value = param + sizeof(*param);      // 让变长值内容存放于 param 紧随其后的内存地址中
    param->length = length;

    memcpy(param->value, value, length);

    return param;
}