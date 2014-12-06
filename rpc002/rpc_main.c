/**
 * 这是 RPC 示例的启动程序。
 * 它根据配置内容，通过 form 启动其它服务结点，
 * 随后自动转换成 WatchDog，负责监控和重启其它结点。
 */
#include <stdio.h>

#include "rpc_log.h"

int main(int argc, const char *argv[])
{
  console_log("%s\n", "hello rpc!");

  return 0;
}
