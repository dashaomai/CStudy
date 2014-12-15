#include <stdlib.h>
#include <st.h>

#include "rpc_queue.h"
#include "rpc_log.h"

void *_queue_task_worker(void *arg);

struct rpc_queue *queue_create(void) {
  struct rpc_queue *queue;
  queue = (struct rpc_queue*)calloc(1, sizeof(struct rpc_queue));

  return queue;
}

void queue_free(struct rpc_queue *queue) {
  // TODO: 还需要遍历链表，将现队列里每个 struct rpc_package_head 结点都释放掉
  struct rpc_package_head *p;

  for (p = queue->first; p != NULL; p = p->next) {
    protocol_package_free(p);
  }
  free(queue);
}

void queue_put(struct rpc_queue *queue, struct rpc_package_head *task) {
  struct rpc_package_head *tail;

  tail = queue->last;

  if (tail != NULL) {
    assert(tail->next == NULL);

    tail->next = task;
  } else {
    queue->head = task;
  }

  queue->last = task;
  queue->count ++;
}

struct rpc_package_head *queue_get(struct rpc_queue *queue) {
  struct rpc_package_head *head;

  head = queue->first;

  if (head != NULL) {
    queue->head = head->next;
    queue->count--;

    if (queue->count == 0)
      queue->last = NULL;

    head->next = NULL;
  }

  return head;
}


void queue_schedule(struct rpc_queue *queue) {
  // TODO: 添加线程数量限制逻辑
  struct rpc_package_head *task;
  task = queue_get(queue);

  if (task != NULL) {
    st_thread_create(_queue_task_worker, task, 0, 0);
    queue->active++;
  }
}


void *_queue_task_worker(void *arg) {
  // TODO: 具体 struct rpc_package_head 的业务处理过程
}
