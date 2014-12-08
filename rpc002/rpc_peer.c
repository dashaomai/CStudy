#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "rpc_peer.h"
#include "rpc_log.h"

int peer_create(struct peer_info *dest, const peer_index_t index, const char *name, const int port) {
  dest->index = index;
  memcpy(dest->name, name, strlen(name));

  extern peer_index_t self_index;

  if (index != self_index)
    return 0;

  // 现在开始创建 rpc_fd
  int listen_fd;
  struct addrinfo hints, *ai, *p;
  int reuseaddr = 1;
  int rv;
  char rpc_port[16];

  extern peer_index_t peer_count;

  snprintf(rpc_port, sizeof(rpc_port), "%d", port);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  LOG("[%d] 准备从以下地址获取 IP 信息：0.0.0.0:%d\n", self_index, port);

  if ((rv = getaddrinfo("0.0.0.0", rpc_port, &hints, &ai)) != 0) {
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
    ERR("[%d] failed to bind at 0.0.0.0\n", self_index);
    return -1;
  }

  freeaddrinfo(ai);
  ai = NULL;
  p = NULL;

  if (listen(listen_fd, 10) == -1) {
    ERR("[%d] failed to listen in %d\n", self_index, port);
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

  LOG("[%d] count of services is %d.\n", self_index, peer_count);

  return 0;
}
