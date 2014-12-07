#include <stdio.h>
#include <st.h>

#include "rpc_log.h"

#define peer_index_t uint8_t
#define MAX_INDEX sizeof(peer_index_t)

// 结点名字最大长度
#define MAX_NAME  32;

peer_index_t    peer_count;
peer_index_t    self_index;

struct peer_info {
  const peer_index_t  index;
  const char    name[MAX_NAME];
  st_netfd_t    rpc_fd;
};

struct peer_info *peer_list;

/**
 * 向指定的 peer_info 结构体内赋值并准备建立 rpc 通讯
 * @arg   {struct peer_info*}         要被写入的结构体
 * @arg   {const peer_index_t}        该结点的 index
 * @arg   {const char*}               该结点的名称
 * @arg   {const int}                 该结点的 RPC 侦听端口
 */
void peer_create(struct peer_info *dest, const peer_index_t index, const char *name, const int port);
