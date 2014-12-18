#include <stdlib.h>
#include <string.h>

#include "serial.h"

void _serial_encode_value(void *dest, struct parameter *param);
void _serial_decode_value(void *src, struct parameter *param);




struct serial_binary *serial_encode(struct parameter_queue *queue) {
  struct serial_binary *binary;
  uint8_t *bytes;
  uint16_t total_length;

  // 计算整个参数队列编码后的总长度
  total_length = sizeof(total_length); // 首先把两字节长度信息计算在内

  total_length += sizeof(queue->count);

  struct parameter *param;
  for (param = queue->head; param  != NULL; param = param->next) {
    switch (param->type) {
      case ARRAY:
        total_length += sizeof(param->type) + sizeof(param->scala_type) + sizeof(param->length) + sizeof(param->scala_count) + param->length;
        break;

      default:
        total_length += sizeof(param->type) + sizeof(param->length) + param->length;
        break;
    }
  }

  binary = (struct serial_binary *)malloc(sizeof(struct serial_binary) + total_length);
  binary->length = total_length;

  bytes = binary->bytes;

  // 复制参数队列唯一需要序列化的数量值
  uint16_t cursor, scala_offset, array_offset;

  *(uint16_t*)bytes = htonl(total_length);
  cursor = sizeof(total_length);

  bytes[cursor] = queue->count;
  cursor += sizeof(queue->count);

  param = queue->head;

  scala_offset = sizeof(param->type) + sizeof(param->length);
  array_offset = sizeof(param->type) + sizeof(param->length) + sizeof(param->scala_type) + sizeof(param->scala_count);

  for (param = queue->head; param != NULL; param = param->next) {
    // 复制参数头
    switch (param->type) {
      case ARRAY:
        param->length = htons(param->length);
        param->scala_count = htons(param->scala_count);

        memcpy((void*)(bytes + cursor), &param->type, array_offset);
        cursor += array_offset;

        param->length = ntohs(param->length);
        param->scala_count = ntohs(param->scala_count);

        break;

      default:

        param->length = htons(param->length);

        memcpy(bytes + cursor, &param->type, scala_offset);
        cursor += scala_offset;

        param->length = ntohs(param->length);

        break;
    }

    // 复制值（确保字节序）
    _serial_encode_value(bytes + cursor, param);
    cursor += param->length;

  }

  return binary;
}

void _serial_encode_value(void *dest, struct parameter *param) {
  int i, cursor;

  switch (param->type) {
    case INT8:
    case UINT8:
    case BOOLEAN:
    case FLOAT32:
    case UFLOAT32:
    case FLOAT64:
    case UFLOAT64:
    case STRING:
    case ARRAY:
      // 单字节值和浮点数直接复制
      memcpy(dest, param->value, param->length);
      break;

    case INT16:
    case UINT16:
      // 双字节值要用 htons 进行转换
      for (i = 0, cursor = 0; i < param->length / 2; i++, cursor += 2) {
        *(uint16_t*)(dest + cursor) = htons(*(uint16_t*)(param->value + cursor));
      }
      break;

    case INT32:
    case UINT32:
    case INT64:
    case UINT64:
      // 四字节及以上整数值要用 htonl 进行转换
      // 注：八字节整数实际上高四位与低四位也需要互换位置。但这个库目的不在玩真实字节序精确对准，而是能进行序列化与反序列化前后结果对应，不在乎序列化中间结果的内容是否严格正确。所以八字节整数也统统当作四字节整数相同处理。
      for (i = 0, cursor = 0; i < param->length / 4; i++, cursor += 4) {
        *(uint32_t*)(dest + cursor) = htonl(*(uint32_t*)(param->value + cursor));
      }
      break;
  }
}




struct parameter_queue *serial_decode(struct serial_binary *binary) {
  struct parameter_queue *queue;
  struct parameter *param;

  void *bytes;
  // bytes 指针直接指向数据长度之后的二进制数据区
  bytes = binary->bytes + sizeof(uint16_t);

  uint8_t   count, i;
  enum parameter_type type, scala_type;
  uint16_t  cursor, length, scala_count;
  cursor = 0;

  // 读取参数队列内元素总数
  queue = parameter_queue_alloc();

  count = *(uint8_t*)bytes + 1;
  cursor += 1;

  // 逐个解码参数对象
  for (i = 0; i < count; i++) {
    type = *(enum parameter_type*)(bytes + cursor);
    cursor += sizeof(type);

    length = ntohs(*(uint16_t*)(bytes + cursor));
    cursor += sizeof(length);

    // 还是将参数与它的值空间分配到一起
    param = (struct parameter *)calloc(1, sizeof(struct parameter) + length);
    param->type = type;
    param->length = length;
    param->value = (void*)param + sizeof(*param);

    switch (type) {
      case ARRAY:
        param->scala_type = *(enum parameter_type*)(bytes + cursor);
        cursor += sizeof(param->scala_type);

        param->scala_count = ntohs(*(uint16_t*)(bytes + cursor));
        cursor += sizeof(param->scala_count);

        break;

      default:
        break;
    }

    // 解出值（确保字节序）
    _serial_decode_value(bytes + cursor, param);
    cursor += length;

    // 参数解码完成，放入队列当中
    parameter_queue_put(queue, param);
  }

  return queue;
}

void _serial_decode_value(void *src, struct parameter *param) {
  int i, cursor;

  switch (param->type) {
    case INT8:
    case UINT8:
    case BOOLEAN:
    case FLOAT32:
    case UFLOAT32:
    case FLOAT64:
    case UFLOAT64:
    case STRING:
    case ARRAY:
      // 单字节值和浮点数直接复制
      memcpy(param->value, src, param->length);
      break;

    case INT16:
    case UINT16:
      // 双字节值要用 htons 进行转换
      for (i = 0, cursor = 0; i < param->length / 2; i++, cursor += 2) {
        *(uint16_t*)(param->value + cursor) = htons(*(uint16_t*)(src + cursor));
      }
      break;

    case INT32:
    case UINT32:
    case INT64:
    case UINT64:
      // 四字节及以上整数值要用 htonl 进行转换
      // 注：八字节整数实际上高四位与低四位也需要互换位置。但这个库目的不在玩真实字节序精确对准，而是能进行序列化与反序列化前后结果对应，不在乎序列化中间结果的内容是否严格正确。所以八字节整数也统统当作四字节整数相同处理。
      for (i = 0, cursor = 0; i < param->length / 4; i++, cursor += 4) {
        *(uint32_t*)(param->value + cursor) = htonl(*(uint32_t*)(src + cursor));
      }
      break;
  }
}
