#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, const char *argv[])
{
  struct addrinfo hints, *res;
  int status;
  int sockfd;

  if (argc != 3) {
    fprintf(stderr, "usage: conn <hostname> <port>\n");
    return 1;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(argv[1], argv[2], &hints, &res);
  if (status != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == 0) {
    fprintf(stderr, "socket: \n");
    return 3;
  }

  status = connect(sockfd, res->ai_addr, res->ai_addrlen);
  if (status == -1) {
    fprintf(stderr, "connect: %d\n", status);
    return 4;
  }

  freeaddrinfo(res);

  return 0;
}
