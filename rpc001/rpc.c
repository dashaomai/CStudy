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

#include "st.h"

void start_processes(const int procnum);
void create_rpc_listeners(void);

void err_sys_quit(const int fd, const char *fmt, ...);
static void err_doit(int fd, int errnoflag, const char *fmt, va_list ap);
char *err_tstamp(void);

/* rpc 通讯走 TCP/IP 方式 */
#define RPC_PORT 9555

int main(int argc, const char *argv[])
{
  if (st_init() < 0)
    err_sys_quit(0, "st_init");

  start_processes(5);

  create_rpc_listeners();

  return 0;
}

int   vp_count;
int   *fd_list;
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
      if (i == 0) exit(1);
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
      fd_list = (int *)calloc(i, sizeof(int));
    }
  }
}

void create_rpc_listeners(void) {
  fprintf(stderr, "my index: %d\tmy pid: %d\n", my_index, my_pid);

  for (;;);
}

/**
 * 产生严重错误时，输出错误消息并退出进程
 */
void err_sys_quit(const int fd, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(stderr, 1, fmt, ap);
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
