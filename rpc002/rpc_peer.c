#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>

#include "rpc_peer.h"
#include "rpc_log.h"

#include "../beej/common.h"

static void *_interconnect_to_peers(void *arg);
static void *_handle_peer_interconnect(void *arg);

int _peer_listen(const peer_index_t index);
void _peer_accept(void);
int _peer_connect(const peer_index_t index);

void peer_create(struct peer_info *dest, const peer_index_t index, const char *name, const char *host, const int port) {
  dest->index = index;
  memcpy(dest->name, name, strlen(name));
  memcpy(dest->host, host, strlen(host));
  snprintf(dest->port, LONGEST_PORT, "%d", port);
}

int peer_listen_and_interconnect(void) {
  int i, rv;

  // 先创建自己的侦听
  if (_peer_listen(self_index) != 0)
    return -1;

  // 用线程进行到其它结点的互联
  if (st_thread_create(_interconnect_to_peers, NULL, 0, 0) == NULL) {
    ERR("[%d] failed to create the peer link thread: %s\n", self_index, strerror(errno));
    return -1;
  }

  // 主线程回环，用于接收远程 rpc 结点发来的数据事件
  _peer_accept();

  return 0;
}

static void *_interconnect_to_peers(void *arg) {
  // TODO: 通过延时来等待其它结点完成侦听。将来要改
  st_sleep(1);

  int i;

  LOG("[%d] 准备向 %d 个 Peer 建立互联\n", self_index, peer_count);

  for (i = 0; i < peer_count; i++) {
    if (i != self_index)
      if (_peer_connect(i) != 0)
        ERR("[%d] failed to connect to peer #%d: %s\n", self_index, i, strerror(errno));
  }

  LOG("[%d] 互联线程运行完毕\n", self_index);

  return 0;
}

static void *_handle_peer_interconnect(void *arg) {
  assert(arg != NULL);

  LOG("[%d] 收到 rpc 客户端连接\n", self_index);

  st_netfd_t client = *(st_netfd_t*)arg;
  arg = NULL;

  // 握手
  // rpc客户端连入后，会主动发来客户端自己的 index
  // 长度为 1 字节
  char    buf[4096];
  ssize_t  len;

  // 先只读取 1 字节的客户端握手头，表示客户端自己的 index
  if ((len = st_read(client, buf, 1, ST_UTIME_NO_TIMEOUT)) < 0) {
    ERR("[%d] failed to handshake from client #%d: %s\n", self_index, *buf, strerror(errno));
    goto close_fd_and_quit;
  } else if (len == 0) {
    goto close_fd_and_quit;
  }

  uint8_t   client_index = (uint8_t)buf[0];

  LOG("[%d] 来自 rpc 客户端 #%d 的握手已经成功建立\n", self_index, client_index);

  // 如果 client_index 等于自己的 self_index，则这个有问题
  if (client_index == self_index) {
    ERR("[%d] rpc client promet the same index with mine.\n", self_index);
    goto close_fd_and_quit;
  }

  // 将客户端 fd 放入属于它 index 的 fd_list 内
  // 在前面的 make link to peers 当中，已经把写去远程结点的 st_netfd_t 保存于 fd_list 之内了
  // 所以不需要需要将远程连入的 st_netfd_t 保存在自己的 fd_list
  /*if (fd_list[client_index] != NULL) {
    ERR("[%d] This client #%d has connected before, replace it.\n", self_index, client_index);
    st_netfd_close(fd_list[client_index]);
  }
  fd_list[client_index] = client;*/

  // 初始化用于读取流的包结构
  struct rpc_package package;
  memset((void*)&package, 0, sizeof(package));

  const size_t pkghead_len = sizeof(rpcpkg_len);
  size_t      receive;
  size_t      cursor; // 记录数据偏移到了 buf 的什么位置

  // 循环服务处理
  for (;;) {
    if ((len = st_read(client, buf, sizeof(buf), ST_UTIME_NO_TIMEOUT)) < 0) {
      ERR("[%d] failed when read from client #%d.\n", self_index, client_index);
      goto close_fd_and_quit;
    } else if (len == 0) {
      goto close_fd_and_quit;
    } else {
      if (len > sizeof(buf))
        LOG("[%d] read %ld bytes into buffer with size: %lu bytes.\n", self_index, len, sizeof(buf));

      // 流进来数据了
      cursor = 0;
      while (cursor < len) { // 如果缓冲区内数据没有处理完

        // 如果之前没切过包，或者前一包已经完成
        if (package.total == package.received) {
          package.total = NTOH(*(rpcpkg_len *)(buf + cursor));

          if (len - cursor - pkghead_len >= package.total) {
            package.received = package.total;
          } else {
            package.received = len - cursor - pkghead_len;
          }

          memcpy(&package.data, buf + cursor + pkghead_len, package.received);

          cursor += package.received + pkghead_len;
        } else {
          // 现在处理的是断开包
          assert(package.received < package.total);

          receive = (len >= package.total - package.received) ? package.total - package.received : len;

          memcpy(&package.data + package.received, buf + cursor, receive);

          package.received += receive;
          cursor += receive;
        }

        // 如果刚刚处理过的包已经是完整包，则处决它
        if (package.received == package.total) {
          LOG("[%d] receive an rpc request with content: %s\n", self_index, package.data);
          // TODO: 添加收到 rpc 包的业务处理
        }
      }
    }
  }

close_fd_and_quit:
  st_netfd_close(client);
  return 0;
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

void _peer_accept(void) {
  st_netfd_t client;
  struct sockaddr from;
  int fromlen = sizeof(from);
  struct peer_info *peer_info;

  peer_info = &peer_list[self_index];

  // 每个 Peer 的主回环
  for (;;)
    if ((client = st_accept(peer_info->rpc_fd, &from, &fromlen, ST_UTIME_NO_TIMEOUT)) != NULL) {
      LOG("[%d] 收到一个 RPC 互联请求\n", self_index);

      st_netfd_setspecific(client, get_in_addr(&from), NULL);

      if (st_thread_create(_handle_peer_interconnect, &client, 0, 0) == NULL)
        ERR("[%d] failed to create the client thread: %s\n", self_index, strerror(errno));
    }

  LOG("[%d] 完成了当前 Peer 的主循环\n", self_index);

  st_netfd_close(peer_info->rpc_fd);
}

int _peer_connect(const peer_index_t index) {
  int client_fd, i, rv, result;
  struct addrinfo hints, *ai, *p;
  struct peer_info *peer_info;

  result = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  peer_info = &peer_list[index];

  LOG("[%d] 向 Peer #%d %s:%s 建立互联\n", self_index, index, peer_info->host, peer_info->port);

  if ((rv = getaddrinfo(peer_info->host, peer_info->port, &hints, &ai)) != 0) {
    ERR("[%d] failed to getaddrinfo to peer #%d\n", self_index, index);
    return -1;
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    if ((client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    if ((peer_info->rpc_fd = st_netfd_open(client_fd)) == NULL) {
      close(client_fd);
      continue;
    }

    if ((rv = st_connect(peer_info->rpc_fd, p->ai_addr, p->ai_addrlen, ST_UTIME_NO_TIMEOUT)) != 0) {
      st_netfd_close(peer_info->rpc_fd);
      close(client_fd);
      continue;
    }

    // 写入 1 字节的 rpc 握手头
    if ((rv = st_write(peer_info->rpc_fd, (char *)&self_index, 1, ST_UTIME_NO_TIMEOUT)) == -1) {
      ERR("[%d] handshake failed.\n", self_index);
      result = -1;
    }

    break;
  }

  if (p == NULL) {
    ERR("[%d] failed to connect to peer #%d.\n", self_index, index);
    result = -1;
  }

  freeaddrinfo(ai);
  ai = NULL;
  p = NULL;

  return result;
}
