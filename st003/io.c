#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <st.h>

#include "io.h"

void *handle_conn(void *arg);

void create_listener_and_accept(void) {
  st_set_eventsys(ST_EVENTSYS_ALT);
  st_init();

  int listener_fd;
  struct addrinfo hints, *ai, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo("0.0.0.0", "9595", &hints, &ai);

  for (p = ai; p != NULL; p = p->ai_next) {
    listener_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener_fd < 0) continue;

    if (bind(listener_fd, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener_fd);
      continue;
    }

    break;
  }

  if (p == NULL) return;

  listen(listener_fd, 10);

  context = (struct peer_context*)calloc(5, sizeof(struct peer_context));

  context[0].rpc_fd = st_netfd_open_socket(listener_fd);

  st_netfd_t client;

  struct sockaddr from;
  int len = sizeof(from);

  while ((client = st_accept(context[0].rpc_fd, &from, &len, ST_UTIME_NO_TIMEOUT)) != NULL) {
    st_thread_create(handle_conn, &client, 0, 0);
  }
}

void *handle_conn(void *arg) {
  st_netfd_t client;
  client = *(st_netfd_t*)arg;
  arg = NULL;

  char buff[1024];
  int len = sizeof(buff) / sizeof(buff[0]);

  int received;

  received = st_read(client, buff, len, ST_UTIME_NO_TIMEOUT);

  fprintf(stdout, "%s\n", buff);

  received = st_write(client, buff, received, ST_UTIME_NO_TIMEOUT);

  st_netfd_close(client);

  return 0;
}
