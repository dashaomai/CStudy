/*
 * 一个 FIFO 队列。
 * 用作 RPC 结点内的任务队列。
 */
#include <stdint.h>

#include "rpc_protocol.h"

struct rpc_queue {
  uint16_t                count;    // 任务总数量
  uint16_t                active;   // 正在线程中处理的数量
  struct rpc_package_head *first;
  struct rpc_package_head *last;
};

struct rpc_queue *queue_create(void);
void queue_free(struct rpc_queue *queue);

void queue_put(struct rpc_queue *queue, struct rpc_package_head *task);
struct rpc_package_head *queue_get(struct rpc_queue *queue);

void queue_schedule(struct rpc_queue *queue);
