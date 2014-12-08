/**
 * 服务结点。
 * 一个结点是一个独立的进程，它和其它所有结点建立两条单向的 TCP/IP 连接，
 * 并读取来自其它结点的 RPC 请求数据包，将其放入自己的任务队列当中。
 */

#include <st.h>

#define peer_index_t uint8_t
#define MAX_INDEX_OF_PEER sizeof(peer_index_t)

// 结点名字最大长度
#define LONGEST_NAME_OF_PEER  32

#define LONGEST_HOST 16
#define LONGEST_PORT 6

peer_index_t    peer_count;
peer_index_t    self_index;

struct peer_info {
  peer_index_t  index;
  char          name[LONGEST_NAME_OF_PEER];
  char          host[LONGEST_HOST];
  char          port[LONGEST_PORT];
  st_netfd_t    rpc_fd;
};

struct peer_info *peer_list;

// 下面是 RPC 通讯包的相关定义
#define rpcpkg_len uint16_t
#define HTON(v) htons(v)
#define NTOH(v) ntohs(v)

struct rpc_package {
  rpcpkg_len  total;
  rpcpkg_len  received;
  uint8_t     data[0xFFFF];  // 64k buffer
};

static struct rpc_package *package;

/**
 * 向指定的 peer_info 结构体内赋值并准备建立 rpc 通讯
 * @arg   {struct peer_info*}         要被写入的结构体
 * @arg   {const peer_index_t}        该结点的 index
 * @arg   {const char*}               该结点的名称
 * @arg   {const char*}               该结点的侦听主机地址
 * @arg   {const int}                 该结点的 RPC 侦听端口
 * @return  {int}                     是否创建成功的标识。0 为成功创建，-1 为失败
 */
void peer_create(struct peer_info *dest, const peer_index_t index, const char *name, const char *host, const int port);

/**
 * 指定一个 index，创建对应它的 peer_info 侦听或连接
 * @return  {int}                     是否创建成功的标识。0 为成功创建，-1 为失败
 */
int peer_listen_and_interconnect(void);
