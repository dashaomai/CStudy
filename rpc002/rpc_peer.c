#include "rpc_peer.h"

void peer_create(struct peer_info *dest, const peer_index_t index, const char *name, const int port) {
  dest->index = index;
  memcpy(dest->name, name, strlen(name));

  // 现在开始创建 rpc_fd
  int listen_fd;
  struct addrinfo hints, *ai, *p;
  int reuseaddr = 1;
  int rv;
  char rpc_port[16];

  snprintf(rpc_port, sizeof(rpc_port), "%d", port);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  LOG("[%d] 准备从以下地址获取 IP 信息：0.0.0.0:%d\n", self_index, port);

  if ((rv = getaddrinfo("0.0.0.0", rpc_port, &hints, &ai)) != 0) {
    ERR("[%d] getaddrinfo 时出现错误：%s\n", self_index, strerror(errno));
    exit(1);
  }

  for (p = ai; p != NULL, p = p->ai_next) {
  }

  freeaddrinfo(ai);
  ai = NULL;
  p = NULL;
}
