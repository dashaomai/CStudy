/**
 * 这是 RPC 示例的启动程序。
 * 它根据配置内容，通过 form 启动其它服务结点，
 * 随后自动转换成 WatchDog，负责监控和重启其它结点。
 */
#include <stdio.h>
#include <stdlib.h>

#include "rpc_peer.h"
#include "rpc_log.h"

void start_processes(void);

int main(int argc, const char *argv[])
{
  start_processes();

  peer_listen_and_interconnect();

  return 0;
}

void start_processes(void) {
  // TODO: 目前是手写的配置，将来需要改为外部配置
  const char *services[] = { "master", "connector", "chat", "login" };
  const char *hosts[] = { "0.0.0.0", "0.0.0.0", "0.0.0.0", "0.0.0.0" };
  const int portes[] = { 9555, 9556, 9557, 9558 };
  int i;
  pid_t pid;

  extern peer_index_t self_index;
  extern peer_index_t peer_count;
  extern struct peer_info *peer_list;

  // 取出数组长度
  self_index = 0;
  peer_count = sizeof(portes) / sizeof(portes[0]);

  for (i = 1; i < peer_count; i++) {
    if ((pid = fork()) < 0) {
      ERR("[%d] 创建子进程失败了！\n", i);
      exit(1);
    } else if (pid == 0) {
      // child
      self_index = i;

      break;
    } else {
      // parent
    }
  }

  // 在主和子进程内，创建 peer_list
  LOG("[%d] 准备创建结点的列表\n", self_index);

  peer_list = (struct peer_info *)calloc(peer_count, sizeof(struct peer_info));

  for (i = 0; i < peer_count; i++) {
    peer_create(peer_list + i, i, services[i], hosts[i], portes[i]);
  }
}
