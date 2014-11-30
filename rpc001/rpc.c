/**
 *
 * rpc 这个程序是作为 rpc 基础机制的第一个练手项目。
 *
 * 这是一个多进程架构，主程序 rpc 启动后，将自动转换为 watchdog，
 * 并创建两个子进程 A、B。每个子进程之间可以互相做 rpc 调用。
 *
 * 初期 rpc 调用查询表都内置于程序代码内部，不走外部调用
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <st.h>

#include "../beej/common.h"

void start_processes(const int procnum);
void create_rpc_listeners(void);
void accept_client(void);

static void *handle_clientconn(void *arg);

void err_sys_quit(const int fd, const char *fmt, ...);
static void err_doit(int fd, int errnoflag, const char *fmt, va_list ap);
char *err_tstamp(void);

/* rpc 通讯走 TCP/IP 方式 */
#define RPC_PORT 9555

int main(int argc, const char *argv[])
{
  start_processes(5);

  create_rpc_listeners();

  accept_client();

  return 0;
}

int   vp_count;

st_netfd_t   *fd_list;
st_netfd_t   rpc_fd;

int   my_index;
pid_t my_pid;

/**
 * 启动工作进程
 */
void start_processes(const int procnum)
{
  int i, status;
  pid_t pid;
  sigset_t mask, omask;

  my_index = 0;
  my_pid = getpid();

  for (i = 1; i <= procnum; i++) {
    if ((pid = fork()) < 0) {
      fprintf(stderr, "fork\n");
      if (i == 1) exit(1);
      fprintf(stderr, "WARN: started only %d processes out of %d\n", i, procnum);
      vp_count = i;

      break;
    } else if (pid == 0) {
      // child
      my_index = i;
      my_pid = getpid();

      break;
    } else {
      // parent, or master
      vp_count = i;
    }
  }
}

/**
 * 创建用于 rpc 通讯的 TCP/IP 内容
 */
void create_rpc_listeners(void) {
  fprintf(stderr, "my index: %d\tmy pid: %d\n", my_index, my_pid);

  int fd;
  struct addrinfo hints, *ai, *p;

  int reuseaddr = 1; // for SO_REUSEADDR
  int rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int rpc_port = RPC_PORT + my_index;
  char port[16];

  sprintf(port, "%d", rpc_port);

  if ((rv = getaddrinfo("0.0.0.0", port, &hints, &ai)) != 0) {
    err_sys_quit(1, "getaddrinfo");
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd < 0) continue;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

    if (bind(fd, p->ai_addr, p->ai_addrlen) < 0) {
      close(fd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    err_sys_quit(1, "failed to bind at 0.0.0.0\n");
  }

  freeaddrinfo(ai); // all done with this
  // 防止野指针
  ai = NULL;
  p = NULL;

  if (listen(fd, 10) == -1) {
    err_sys_quit(1, "failed to listen in %d\n", rpc_port);
  }

  // 初始化 st 线程
  if (st_init() < 0) {
    err_sys_quit(1, "failed to init st\n");
  }

  // 转换 fd
  if ((rpc_fd = st_netfd_open(fd)) == NULL)
    err_sys_quit(1, "failed to convert fd into st_fd: %s\n", strerror(errno));

  fd_list = (st_netfd_t*)calloc(vp_count, sizeof(st_netfd_t));
  fd_list[my_index] = rpc_fd;
}

/**
 * 接收 rpc 连接
 */
void accept_client(void) {
  st_netfd_t client;
  struct sockaddr from;
  int fromlen = sizeof(from);

  while ((client = st_accept(rpc_fd, &from, &fromlen, ST_UTIME_NO_TIMEOUT)) != NULL) {
    st_netfd_setspecific(client, get_in_addr(&from), NULL);

    if (st_thread_create(handle_clientconn, &client, 0, 0) == NULL)
      fprintf(stderr, "failed to create the client thread: %s\n", strerror(errno));
  }

  st_netfd_close(rpc_fd);
}

#define rpcpkg_len uint16_t
#define HTON(v) htons(v)
#define NTOH(v) ntohs(v)

struct rpc_package {
  rpcpkg_len  total;
  rpcpkg_len  received;
  uint8_t     data[sizeof(rpcpkg_len)];
};

static struct rpc_package *package;

static void *handle_clientconn(void *arg) {
  st_netfd_t client = *(st_netfd_t*)arg;
  arg = NULL;

  // 握手
  // rpc客户端连入后，会主动发来客户端自己的 index
  // 长度为 1 字节
  char    buf[4096];
  ssize_t  len;

  // 先只读取 1 字节的客户端握手头，表示客户端自己的 index
  if ((len = st_read(client, buf, 1, ST_UTIME_NO_TIMEOUT)) < 0) {
    fprintf(stderr, "failed to handshake from client: %s\n", strerror(errno));
    goto close_fd_and_quit;
  } else if (len == 0) {
    goto close_fd_and_quit;
  }

  uint8_t   client_index = (uint8_t)buf[0];

  fprintf(stdout, "来自 rpc 客户端 #%d 的握手已经成功建立\n", client_index);

  // 如果 client_index 等于自己的 my_index，则这个有问题
  if (client_index == my_index) {
    fprintf(stderr, "rpc client promet the same index with mine.\n");
    goto close_fd_and_quit;
  }

  // 将客户端 fd 放入属于它 index 的 fd_list 内
  if (fd_list[client_index] != NULL) {
    fprintf(stderr, "This client #%d has connected before, replace it.\n", client_index);
    st_netfd_close(fd_list[client_index]);
  }
  fd_list[client_index] = client;

  // 初始化用于读取流的包结构
  struct rpc_package package;
  memset((void*)&package, 0, sizeof(package));

  const size_t pkghead_len = sizeof(rpcpkg_len);
  size_t      receive;
  size_t      cursor; // 记录数据偏移到了 buf 的什么位置

  // 循环服务处理
  for (;;) {
    if ((len = st_read(client, buf, sizeof(buf), ST_UTIME_NO_TIMEOUT)) < 0) {
      fprintf(stderr, "failed when read from client #%d.\n", client_index);
      goto close_fd_and_quit;
    } else if (len == 0) {
      goto close_fd_and_quit;
    } else {
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

          memcpy(&package.data + package.received, buf + cursor + pkghead_len, receive);

          package.received += receive;
          cursor += receive;
        }

        // 如果刚刚处理过的包已经是完整包，则处决它
        if (package.received == package.total) {
          fprintf(stdout, "receive an rpc request with content: %s\n", package.data);
        }
      }
    }
  }

close_fd_and_quit:
  st_netfd_close(client);
  return 0;
}

/**
 * 产生严重错误时，输出错误消息并退出进程
 */
void err_sys_quit(const int fd, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, 1, fmt, ap);
  va_end(ap);
  exit(1);
}

/**
 * 打印错误文字消息于指定 fd 上，
 * 随后返回调用者
 */
static void err_doit(int fd, int errnoflag, const char *fmt, va_list ap)
{
  int errno_save;
  char buf[512];

  errno_save = errno;
  strcpy(buf, err_tstamp());
  vsprintf(buf + strlen(buf), fmt, ap);

  if (errnoflag)
    sprintf(buf + strlen(buf), fmt, ap);
  else
    strcat(buf, "\n");

  write(fd, buf, strlen(buf));
  errno = errno_save;
}

/**
 * 输出一个当前时间的字符串表示
 */
char *err_tstamp(void)
{
  static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  static char str[32];
  static time_t lastt = 0;

  struct tm *tmp;
  time_t currt = st_time();

  if (currt == lastt)
    return str;

  tmp = localtime(&currt);
  sprintf(str, "[%02d/%s/%d:%02d:%02d:%02d] ", tmp->tm_mday,
          months[tmp->tm_mon], 1900 + tmp->tm_year, tmp->tm_hour,
          tmp->tm_min, tmp->tm_sec);
  lastt = currt;

  return str;
}
