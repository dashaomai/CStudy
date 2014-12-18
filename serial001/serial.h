/**
 * 这是序列化逻辑所在
 */
#include <stdint.h>

#include "parameter.h"

struct serial_binary {
  uint16_t    length;
  void        *bytes;
};

struct serial_binary *serial_encode(struct parameter_queue *queue);
struct parameter_queue *serial_decode(struct serial_binary*bytes);
