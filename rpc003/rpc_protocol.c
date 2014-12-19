#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>

#include "rpc_protocol.h"
#include "rpc_serial.h"
#include "rpc_log.h"

char *malloc_and_copy(const char *src, const rpcpkg_len start, const rpcpkg_len length) {
  char *buffer = (char *)malloc(length + 1);
  memcpy(buffer, &src[start], length);
  buffer[length] = '\0';

  return buffer;
}


char *protocol_encode(const struct rpc_package_head *head, rpcpkg_len *cursor_result) {
  rpcpkg_len pkg_len, temp_len, cursor;
  char *data;

  switch (head->type) {
    case UNKNOW:
      return NULL;

    case REQUEST:
      pkg_len = sizeof(rpcpkg_len) + sizeof(head->type) + sizeof(head->source) + sizeof(head->destination) + sizeof(head->id) + head->body->request.method_len;
      break;

    case RESPONSE:
      pkg_len = sizeof(rpcpkg_len) + sizeof(head->type) + sizeof(head->source) + sizeof(head->destination) + sizeof(head->id);
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
  struct rpc_response *response;

  request = &head->body->request;
  response = &head->body->response;

  struct serial_binary *binary;

  switch (head->type) {
    case UNKNOW:
      break;

    case REQUEST:
      memcpy(data + cursor, &request->method_len, sizeof(request->method_len));
      cursor += sizeof(request->method_len);

      binary = serial_encode(request->queue);

      memcpy(data + cursor, request->method, request->method_len);
      cursor += request->method_len;
      memcpy(data + cursor, binary->bytes, binary->length);
      cursor += binary->length;

      free(binary);

      break;

    case RESPONSE:
      binary = serial_encode(response->queue);

      memcpy(data + cursor, binary->bytes, binary->length);
      cursor += binary->length;

      free(binary);

      break;
  }

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
  len_of_head = sizeof(head->type) + sizeof(head->source) + sizeof(head->destination) + sizeof(head->id);

  memcpy(head, package->data, len_of_head);

  rpcpkg_len    cursor;
  cursor = len_of_head;

  struct rpc_request *request = &head->body->request;
  struct rpc_response *response = &head->body->response;
  struct serial_binary *binary;

  switch (head->type) {
    case UNKNOW:
      break;

    case REQUEST:
      request->method_len = (uint8_t )package->data[cursor];
      cursor += sizeof(request->method_len);

      request->method = malloc_and_copy((const char*)package->data, cursor, request->method_len);
      cursor += request->method_len;

      binary = (struct serial_binary*)malloc(sizeof(struct serial_binary));
      binary->length = package->total - cursor;
      binary->bytes = (void*)(package->data + cursor);

      request->queue = serial_decode(binary);

      free(binary);

      break;

    case RESPONSE:
      binary = (struct serial_binary*)malloc(sizeof(struct serial_binary));
      binary->length = package->total - cursor;
      binary->bytes = (void*)(package->data + cursor);

      response->queue = serial_decode(binary);

      free(binary);

      break;
  }

  return head;
}

struct rpc_package_head *protocol_package_create(enum rpc_package_type type, const peer_index_t source, const peer_index_t destination, const uint8_t id, const char *method, struct parameter_queue *parameters) {
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

      request->method = malloc_and_copy(method, 0, request->method_len);
      request->queue = parameters;

      break;

    case RESPONSE:
      response = &(head->body->response);

      response->queue = parameters;

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
      parameter_queue_free(head->body->request.queue);
      break;

    case RESPONSE:
      parameter_queue_free(head->body->response.queue);
      break;
  }

  free(head->body);
  free(head);
}
