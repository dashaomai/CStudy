#include <stdlib.h>
#include <st.h>
#include <assert.h>

#include "rpc_queue.h"
#include "rpc_log.h"

struct rpc_queue *queue_create(void) {
  struct rpc_queue *queue;
  queue = (struct rpc_queue*)calloc(1, sizeof(struct rpc_queue));

  return queue;
}

void queue_free(struct rpc_queue *queue) {
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
    queue->first = task;
  }

  queue->last = task;
  queue->count ++;
}

struct rpc_package_head *queue_get(struct rpc_queue *queue) {
  struct rpc_package_head *head;

  head = queue->first;

  if (head != NULL) {
    queue->first = head->next;
    queue->count--;

    if (queue->count == 0)
      queue->last = NULL;

    head->next = NULL;
  }

  return head;
}

