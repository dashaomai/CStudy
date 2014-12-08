#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "rpc_peer.h"
#include "rpc_log.h"

int _peer_listen(const peer_index_t index);
int _peer_connect(const peer_index_t index);

void peer_create(struct peer_info *dest, const peer_index_t index, const char *name, const char *host, const int port) {
  dest->index = index;
  memcpy(dest->name, name, strlen(name));
  memcpy(dest->host, host, strlen(host));
  snprintf(dest->port, LONGEST_PORT, "%d", port);
}

int peer_listen_and_interconnect(void) {
  int i, result, rv;

  if ((result = _peer_listen(self_index)) != 0)
    return result;

  // TODO: 通过延时来等待其它结点完成侦听。将来要改
  sleep(1);

  for (i = 0; i < peer_count; i++) {
    if (i != self_index)
      if ((rv = _peer_connect(i)) != 0)
        result = rv;
  }

  return result;
}

int _peer_listen(const peer_index_t index) {
  // 现在开始创建 rpc_fd
  int listen_fd;
  struct addrinfo hints, *ai, *p;
  int reuseaddr = 1;
  int rv;
  struct peer_info *peer_info;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  peer_info = &peer_list[index];

  LOG("[%d] 准备从以下地址获取 IP 信息：%s:%s\n", self_index, peer_info->host, peer_info->port);

  if ((rv = getaddrinfo(peer_info->host, peer_info->port, &hints, &ai)) != 0) {
    ERR("[%d] getaddrinfo 时出现错误：%s\n", self_index, strerror(errno));
    return -1;
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    if ((listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      ERR("[%d] socket: %s\n", self_index, strerror(errno));
      continue;
    }

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

    if (bind(listen_fd, p->ai_addr, p->ai_addrlen) < 0) {
      close(listen_fd);
      ERR("[%d] bind: %s\n", self_index, strerror(errno));
      continue;
    }

    LOG("[%d] success to bind.\n", self_index);
    break;
  }

  if (p == NULL) {
    ERR("[%d] failed to bind at %s.\n", self_index, peer_info->host);
    return -1;
  }

  freeaddrinfo(ai);
  ai = NULL;
  p = NULL;

  if (listen(listen_fd, 10) == -1) {
    ERR("[%d] failed to listen in %d\n", self_index, peer_info->port);
    return -1;
  }

  // 初始化 state-threads 系统
  if (st_set_eventsys(ST_EVENTSYS_ALT) == -1)
    ERR("[%d] Can't set event system to alt: %s\n", self_index, strerror(errno));

  if (st_init() < 0) {
    ERR("[%d] st_init \n", self_index);
    return -1;
  }

  LOG("[%d] The event system of state-threads is: %s\n", self_index, st_get_eventsys_name());

  // 转换 fd
  if ((peer_list[self_index].rpc_fd = st_netfd_open(listen_fd)) == NULL) {
    ERR("[%d] st_netfd_open: %s\n", self_index, strerror(errno));
    return -1;
  }

  return 0;
}

int _peer_connect(const peer_index_t index) {
  int i;

  return 0;
}
