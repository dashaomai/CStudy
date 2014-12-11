#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>

#include "rpc_protocol.h"

char *malloc_and_copy(const char *src, const rpcpkg_len start, const rpcpkg_len length) {
  char *buffer = (char *)malloc(length);
  memcpy(buffer, src + start, length);

  return buffer;
}

const struct rpc_package_head *protocol_decode(const struct rpc_package *package) {
  assert(package->total == package->received);

  struct rpc_package_head *head;

  head = (struct rpc_package_head*)calloc(1, sizeof(struct rpc_package_head));
  head->body = (union rpc_package_body*)calloc(1, sizeof(union rpc_package_body));

  rpcpkg_len    len_of_head, len_of_body;
  len_of_head = sizeof(struct rpc_package_head) - sizeof(union rpc_package_body*);
  len_of_body = package->total - len_of_head;

  memcpy(head, package->data, len_of_head);
  memcpy(head->body, package->data + len_of_head, len_of_body);

  struct rpc_request *request = &head->body->request;
  struct rpc_response *response = &head->body->response;

  switch (head->type) {
    case REQUEST:
      request->parameter_len = ntohs(request->parameter_len);

      request->method = malloc_and_copy((const char*)package->data, len_of_head + sizeof(request->method_len) + sizeof(request->parameter_len), request->method_len);
      request->parameter = malloc_and_copy((const char*)package->data, len_of_head + sizeof(request->method_len) + sizeof(request->parameter_len), request->parameter_len);

      break;

    case RESPONSE:
      response->result_len = ntohs(response->result_len);

      response->result = malloc_and_copy((const char*)package->data, len_of_head + sizeof(response->result_len), response->result_len);

      break;
  }

  return head;
}

struct rpc_package_head *protocol_package_create(enum rpc_package_type type, const peer_index_t source, const peer_index_t destination, const uint8_t id, const char *method, const char *parameter) {
  struct rpc_package_head *head;

  head = (struct rpc_package_head*)calloc(1, sizeof(struct rpc_package_head));
  head->body = (union rpc_package_body*)calloc(1, sizeof(union rpc_package_body));

  head->type = type;
  head->source = source;
  head->destination = destination;
  head->id = id;

  struct rpc_request *request;
  struct rpc_response *response;

  switch (type) {
    case REQUEST:
      request = &head->body->request;

      request->method_len = strlen(method);
      request->parameter_len = strlen(parameter);

      request->method = malloc_and_copy(method, 0, request->method_len);
      request->parameter = malloc_and_copy(parameter, 0, request->parameter_len);

      break;

    case RESPONSE:
      response = &(head->body->response);

      response->result_len = strlen(parameter);
      response->result = malloc_and_copy(parameter, 0, response->result_len);

      break;
  }

  return head;
}
