#include <st.h>

struct peer_context {
  int     index;
  char    name[64];
  st_netfd_t  rpc_fd;
};

struct peer_context *context;

void create_listener_and_accept(void);
