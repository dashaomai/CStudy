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
 * ����һ�� PER_SESSION_CONTEXT���Լ����ڲ��� PER_IO_CONTEXT
 */
LPPER_SESSION_CONTEXT CreateSessionContext(SOCKET socket, SOCKADDR_STORAGE *address, SocketAction action)
{
	LPPER_SESSION_CONTEXT session = NULL;

	session = (LPPER_SESSION_CONTEXT)GlobalAlloc(GPTR, sizeof(PER_SESSION_CONTEXT));

	session->clientSocket = socket;
	// ����Ӧ�����ݽṹ���ݸ��Ƶ��Լ�����õ��ڴ���У��Լ����ڴ���Ƭ�Ŀ�����
	memcpy(&session->clientAddress, address, sizeof(address));

	LPPER_IO_CONTEXT io = &session->ioContext;
	io->buffer.len = BUFFER_SIZE;
	io->buffer.buf = io->bufferStub;
	io->action = action;

	return session;
}