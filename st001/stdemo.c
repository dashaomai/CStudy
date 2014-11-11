#include <stdio.h>
#include <st.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "../beej/common.h"

#define HOST "0.0.0.0"
#define PORT "9500"

#define SEC2USEC(i) i*1000000L

static void *handle_listener(void *arg);
static void *handle_clientconn(void *arg);

int main(int argc, const char *argv[])
{
  // Server 的 socket fd
  int listener_fd;
  st_netfd_t listener;

  int reuseaddr = 1; // for setsockopt() SO_REUSEADDR
  int rv;

  struct addrinfo hints, *ai, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // 使用 IPv4 协议
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // 收取指定地址信息
  if ((rv = getaddrinfo(HOST, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 绑定
  for (p = ai; p != NULL; p = p->ai_next) {
    listener_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener_fd < 0) {
      continue;
    }

    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

    if (bind(listener_fd, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener_fd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "failed to bind in %s\n", HOST);
    return 2;
  }

  freeaddrinfo(ai); // all done with this
  ai = NULL;
  p = NULL;

  // 侦听
  if (listen(listener_fd, 10) == -1) {
    fprintf(stderr, "failed to listen in %s\n", PORT);
    return 3;
  }

  // 初始化多线程
  if (st_init() < 0) {
    fprintf(stderr, "ERROR: initialize of st failed.\n");
    return 4;
  }

  // 转换 fd
  if ((listener = st_netfd_open(listener_fd)) == NULL) {
    fprintf(stderr, "failed to convert fd into st_fd: %s\n", gai_strerror(errno));
    return 5;
  };

  // 将服务器 fd 转入 st thread 内处理
  /*if (st_thread_create(handle_listener, &listener, 0, 0) == NULL) {
    fprintf(stderr, "failed to create the listener thread: %s\n", gai_strerror(errno));
    return 6;
  }*/
  handle_listener(&listener);

  return 0;
}

// 服务器 listener 不停接收客户端接入的地方
static void *handle_listener(void *arg) {
  st_netfd_t listener = *(st_netfd_t *)arg;
  arg = NULL;
  st_netfd_t client;
  struct sockaddr from;
  int fromlen = sizeof(from);

  while ((client = st_accept(listener, &from, &fromlen, ST_UTIME_NO_TIMEOUT)) != NULL) {
    st_netfd_setspecific(client, get_in_addr(&from), NULL);

    if (st_thread_create(handle_clientconn, &client, 0, 0) == NULL) {
      fprintf(stderr, "failed to create the client thread: %s\n", gai_strerror(errno));
    }
  }

  st_netfd_close(listener);

  return 0;
}

// 服务器取得客户端连接后分配给客户端的独立 ST Thread
static void *handle_clientconn(void *arg) {
  st_netfd_t client = *(st_netfd_t *)arg;
  arg = NULL;

  char buf[256];
  static char resp[]= "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n"
                      "Connection: close\r\n\r\n<H2>It works!</H2>\n";
  int n = sizeof(resp) - 1;

  if (st_read(client, buf, sizeof buf, SEC2USEC(10)) < 0) {
    fprintf(stderr, "failed to read from client: %s\n", gai_strerror(errno));
    st_netfd_close(client);
    return 0;
  }

  if (st_write(client, resp, n, ST_UTIME_NO_TIMEOUT) != n) {
    fprintf(stderr, "failed to write to client: %s\n", gai_strerror(errno));
    st_netfd_close(client);
    return 0;
  }

  st_netfd_close(client);
  return 0;
}
