/**
 * 这是 RPC 通讯协议相关数据结构及逻辑
 */
#include <stdint.h>

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
  union       content {
    uint8_t     data[0xFFFF];  // 64k buffer
    struct      package {
      uint8_t   type;
      peer_index_t  source;
      peer_index_t  destination;
      uint8_t   id;
      union     body {
        struct  request {
          char    method[LONGEST_METHOD];
          char    parameter[LONGEST_PARAMETER];
        } request;
        struct  response {
          char    result[LONGEST_RESULT];
        } response;
      } body;
    } package;
  } content;
};

const char *protocol_decode(const char *bytes, const rpcpkg_len len);
