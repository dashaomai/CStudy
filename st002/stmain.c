/**
 * test the fault in st_thread with printf call
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <st.h>

void *handle_accept(void *arg);

int main(int argc, const char *argv[])
{
  st_set_eventsys(ST_EVENTSYS_ALT);
  st_init();

  int listener_fd, client_fd;
  int reuseaddr = 1;

  struct addrinfo hints, *ai, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo("0.0.0.0", "9595", &hints, &ai);

  for (p = ai; p != NULL; p = p->ai_next) {
    listener_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener_fd < 0) continue;

    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    if (bind(listener_fd, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener_fd);
      continue;
    }

    break;
  }

  if (p == NULL) return 1;

  st_netfd_t  listener, client;
  listener = st_netfd_open(listener_fd);

  st_thread_create(handle_accept, &listener, 0, 0);

  client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
  client= st_netfd_open(client_fd);

  if (st_connect(client, p->ai_addr, p->ai_addrlen, ST_UTIME_NO_TIMEOUT) != 0) {
    st_netfd_close(client);
    close(client_fd);

    return 2;
  }

  freeaddrinfo(ai);
  ai = NULL;
  p = NULL;

  char message[] = "hello st!";
  size_t len = sizeof(message);

  st_write(client, message, len, ST_UTIME_NO_TIMEOUT);

  for (; st_sleep(1) == 0;) ;

  st_netfd_close(client);
  st_netfd_close(listener);
  return 0;
}

void *handle_accept(void *arg) {
  st_netfd_t listener = *(st_netfd_t*)arg;
  st_netfd_t client;

  struct sockaddr from;
  size_t len = sizeof(from);

  char buff[1024];
  size_t bufflen = sizeof(buff);

  while ((client = st_accept(listener, &from, &len, ST_UTIME_NO_TIMEOUT)) != NULL) {
    st_read(client, buff, bufflen, ST_UTIME_NO_TIMEOUT);

    fprintf(stdout, "%s\n", buff);
  }
  return NULL;
}
