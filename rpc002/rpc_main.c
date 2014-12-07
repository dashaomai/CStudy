/**
 * 这是 RPC 示例的启动程序。
 * 它根据配置内容，通过 form 启动其它服务结点，
 * 随后自动转换成 WatchDog，负责监控和重启其它结点。
 */
#include <stdio.h>

#include "rpc_log.h"

void start_processes(void);

int main(int argc, const char *argv[])
{
  start_processes();

  return 0;
}

void start_processes(void) {
  const char *services[] = { "master", "connector", "chat", "login" };
  const int portes[] = { 9555, 9556, 9557, 9558 };
  int i;
  pid_t pid;

  for (i = 0; i < sizeof(services); i++) {
    if ((pid = fork()) < 0) {
      ERR("创建子进程失败了！");
      exit(1);
    } else if (pid == 0) {
      // child
      self_index = i + 1;

      break;
    } else {
      // parent
      self_index = 0;
    }
  }

  // 创建 peer_list
  peer_list = (struct peer_info *)calloc(sizeof(services) + 1, sizeof(struct peer_info));

  for (i = 0; i <= sizeof(services); i++) {
    peer_create(peer_list + i, i, services[i-1], portes[i-1]);
  }
}
