/**
 * 这是 RPC 通讯协议相关数据结构及逻辑
 */
#ifndef _RPC_PROTOCOL_H
#define _RPC_PROTOCOL_H

#include <stdint.h>

#include "rpc_parameter.h"

#define peer_index_t uint8_t
#define MAX_INDEX_OF_PEER sizeof(peer_index_t)

#define rpcpkg_len uint16_t
#define HTON(v) htons(v)
#define NTOH(v) ntohs(v)

#define LONGEST_METHOD 0xFF
#define LONGEST_PARAMETER 0xFF00  // 0xFFFF - 0xFF
#define LONGEST_RESULT    0xFFFF

/**
 * 一个 RPC 包的数据结构
 *
 * 由于 data 属性设定为 64k
 * 如果在堆栈中声明该数据结构，
 * 则在 Linux 下会产生段错误。
 * （测试时在 FreeBSD 和 MacOSX 上没出现这种错误，
 * 应该是默认堆栈空间较大所致）
 *
 * 同时这种堆栈崩溃也会影响 gdb，
 * 让它无法准确定位到出错的行数。
 *
 * 所以运用这个数据结构时，一定要用 malloc 来从堆中分配内存
 * 不能在栈里直接分配
 */
struct rpc_package {
  rpcpkg_len  total;
  rpcpkg_len  received;
  uint8_t     data[0xFFFF];  // 64k buffer
};

/**
 * Request 数据体
 */
struct rpc_request {
  uint8_t   method_len;
  uint16_t  parameter_len;
  char      *method;
  char      *parameter;
};

/**
 * Response 数据体
 */
struct rpc_response {
  uint16_t  result_len;
  char      *result;
};

/**
 * RPC 包的数据体
 */
union rpc_package_body {
  struct rpc_request request;
  struct rpc_response response;
};


enum rpc_package_type {
  UNKNOW = '\0',
  REQUEST,
  RESPONSE
};

/**
 * RPC 包的数据头
 */
struct rpc_package_head {
  enum rpc_package_type type;
  peer_index_t  source;
  peer_index_t  destination;
  uint8_t   id;
  union rpc_package_body *body;
  struct rpc_package_head *next; // 链表支持
};

char *protocol_encode(const struct rpc_package_head *head, rpcpkg_len *cursor_result);
struct rpc_package_head *protocol_decode(const struct rpc_package *package);
struct rpc_package_head *protocol_package_create(enum rpc_package_type type, const peer_index_t source, const peer_index_t distination, const uint8_t id, const char *method, const char *parameter);


void protocol_package_free(struct rpc_package_head *head);

#endif
