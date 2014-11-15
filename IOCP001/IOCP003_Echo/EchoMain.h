#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
// #pragma comment(lib, "Kernel32.lib")

typedef enum SocketAction
{
	SocketAccept,
	SocketRead,
	SocketWrite
} SocketAction;

#define BUFFER_SIZE 1024
typedef struct _PER_IO_CONTEXT
{
	OVERLAPPED		overlapped;
	WSABUF			buffer;
	CHAR			bufferStub[BUFFER_SIZE];
	SocketAction	action;
} PER_IO_CONTEXT, *LPPER_IO_CONTEXT;

typedef struct _PER_SESSION_CONTEXT
{
	SOCKET				clientSocket;
	SOCKADDR_STORAGE	clientAddress;
	PER_IO_CONTEXT		ioContext;
} PER_SESSION_CONTEXT, *LPPER_SESSION_CONTEXT;

/**
 * 创建一个 PER_SESSION_CONTEXT，以及其内部的 PER_IO_CONTEXT
 */
LPPER_SESSION_CONTEXT CreateSessionContext(SOCKET socket, SOCKADDR_STORAGE *address, SocketAction action)
{
	LPPER_SESSION_CONTEXT session = NULL;

	session = (LPPER_SESSION_CONTEXT)GlobalAlloc(GPTR, sizeof(PER_SESSION_CONTEXT));

	session->clientSocket = socket;
	// 将对应的数据结构内容复制到自己分配好的内存块中，以减轻内存碎片的可能性
	memcpy(&session->clientAddress, address, sizeof(address));

	LPPER_IO_CONTEXT io = &session->ioContext;
	io->buffer.len = BUFFER_SIZE;
	io->buffer.buf = io->bufferStub;
	io->action = action;

	return session;
}