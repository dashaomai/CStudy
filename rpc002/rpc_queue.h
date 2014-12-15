/*
 * 一个 FIFO 队列。
 * 用作 RPC 结点内的任务队列。
 */
#include <stdint.h>

#include "rpc_protocol.h"

#ifndef _RPC_QUEUE_H
#define _RPC_QUEUE_H

struct rpc_queue {
  uint16_t                count;    // 任务总数量
  struct rpc_package_head *first;
  struct rpc_package_head *last;
};

struct rpc_queue *queue_create(void);
void queue_free(struct rpc_queue *queue);

void queue_put(struct rpc_queue *queue, struct rpc_package_head *task);
struct rpc_package_head *queue_get(struct rpc_queue *queue);

#endif
