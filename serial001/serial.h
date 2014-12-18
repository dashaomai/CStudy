/**
 * 这是序列化逻辑所在
 */
#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

#include "parameter.h"

struct serial_binary {
  uint16_t    length;
  void        *bytes;
};

struct serial_binary *serial_encode(struct parameter_queue *queue);
struct parameter_queue *serial_decode(struct serial_binary *binary);

#endif
