#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>

#include "rpc_protocol.h"
#include "rpc_log.h"

char *malloc_and_copy(const char *src, const rpcpkg_len start, const rpcpkg_len length) {
  char *buffer = (char *)malloc(length + 1);
  memcpy(buffer, src + start, length);
  buffer[length] = '\0';

  return buffer;
}




char *protocol_encode(const struct rpc_package_head *head, rpcpkg_len *cursor_result) {
  rpcpkg_len pkg_len, temp_len, cursor;
  char *data;

  // pkg_len = sizeof(rpcpkg_len) + sizeof(struct rpc_package_head) - sizeof(union rpc_package_body*) + sizeof(union rpc_package_body);
  switch (head->type) {
    case UNKNOW:
      break;

    case REQUEST:
      pkg_len = sizeof(rpcpkg_len) + sizeof(head->type) + sizeof(head->source) + sizeof(head->destination) + sizeof(head->id) + head->body->request.method_len + head->body->request.parameter_len;
        break;

    case RESPONSE:
        pkg_len = sizeof(rpcpkg_len) + sizeof(head->type) + sizeof(head->source) + sizeof(head->destination) + sizeof(head->id) + head->body->response.result_len;
        break;
  }
  data = (char *)malloc(pkg_len);

  cursor = sizeof(rpcpkg_len);
  memcpy(data + cursor, &head->type, sizeof(head->type));
  cursor += sizeof(head->type);
  memcpy(data + cursor, &head->source, sizeof(head->source));
  cursor += sizeof(head->source);
  memcpy(data + cursor, &head->destination, sizeof(head->destination));
  cursor += sizeof(head->destination);
  memcpy(data + cursor, &head->id, sizeof(head->id));
  cursor += sizeof(head->id);

  struct rpc_request *request;
  request = &head->body->request;

  memcpy(data + cursor, &request->method_len, sizeof(request->method_len));
  cursor += sizeof(request->method_len);
  temp_len = htons(request->parameter_len);
  memcpy(data + cursor, &temp_len, sizeof(request->parameter_len));
  cursor += sizeof(request->parameter_len);

  memcpy(data + cursor, request->method, request->method_len);
  cursor += request->method_len;
  memcpy(data + cursor, request->parameter, request->parameter_len);
  cursor += request ->parameter_len;

  // 写上包长于头部
  temp_len = cursor - sizeof(rpcpkg_len);
  temp_len = htons(temp_len);
  memcpy(data, &temp_len, sizeof(temp_len));

  *cursor_result = cursor;

  return data;
}



struct rpc_package_head *protocol_decode(const struct rpc_package *package) {
  assert(package->total == package->received);

  struct rpc_package_head *head;

  head = (struct rpc_package_head*)calloc(1, sizeof(struct rpc_package_head));
  head->body = (union rpc_package_body*)calloc(1, sizeof(union rpc_package_body));

  rpcpkg_len    len_of_head;
  // len_of_head = sizeof(struct rpc_package_head) - sizeof(union rpc_package_body*);
  len_of_head = sizeof(head->type) + sizeof(head->source) + sizeof(head->destination) + sizeof(head->id);
  // len_of_body = package->total - len_of_head;

  memcpy(head, package->data, len_of_head);

  rpcpkg_len    cursor;
  cursor = len_of_head;

  struct rpc_request *request = &head->body->request;
  struct rpc_response *response = &head->body->response;

  switch (head->type) {
    case UNKNOW:
      break;

    case REQUEST:
      request->method_len = *(uint8_t*)(package->data + cursor);
      cursor += sizeof(request->method_len);
      request->parameter_len = ntohs(*(uint16_t*)(package->data + cursor));
      cursor += sizeof(request->parameter_len);

      request->method = malloc_and_copy((const char*)package->data, cursor, request->method_len);
      cursor += request->method_len;

      request->parameter = malloc_and_copy((const char*)package->data, cursor, request->parameter_len);

      break;

    case RESPONSE:
      response->result_len = ntohs(*(uint16_t*)(package->data + cursor));
      cursor += sizeof(response->result_len);

      response->result = malloc_and_copy((const char*)package->data, cursor, response->result_len);

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
    case UNKNOW:
      break;

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




void protocol_package_free(struct rpc_package_head *head) {
  assert(head->body);

  switch (head->type) {
    case UNKNOW:
      break;

    case REQUEST:
      free(head->body->request.method);
      free(head->body->request.parameter);
      break;

    case RESPONSE:
      free(head->body->response.result);
      break;
  }

  free(head->body);
  free(head);
}
