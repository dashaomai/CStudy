#include <stdio.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")   // WinSock2 静态链接库
#pragma comment(lib, "Kernel32.lib") // IOCP 依赖的内核库

typedef enum WsaAction
{
	WsaAccept,
	WsaWrite,
	WsaRead
} WsaAction;

/**
* 结构体：PER_IO_CONTEXT
* Overlapped I/O 需要使用的结构体，临时记录 IO 数据
*/
#define DataBuffSize 2048 // 2 * 1024 简陋的双字节兼容方案
typedef struct
{
	OVERLAPPED	overlapped;
	WSABUF		databuff;
	DWORD		bytesLength;
	WsaAction	action;
} PER_IO_CONTEXT;

/**
 * 初始化并返回一个 PER_IO_DATA
 */
PER_IO_CONTEXT* InitAnPerIoData(WsaAction action)
{
	PER_IO_CONTEXT* data = NULL;
	// 分配的空间大小除 PER_IO_CONTEXT 结构体本身必须外，还额外包括了 DataBuffSize 大小的字节数组，作为 data->databuff.buf 运用
	// TODO: 这样分配出的内存空间，会被自动保护，不被其它请求重叠吗？
	data = (PER_IO_CONTEXT*)GlobalAlloc(GPTR, sizeof(PER_IO_CONTEXT) + DataBuffSize * sizeof(char));

	if (data != NULL)
	{
		data->databuff.len = 1024;
		data->databuff.buf = (char*)(&data->databuff + sizeof(data->databuff.len));		// TODO: 这样直接指定指针是否有危险性？
		data->bytesLength = 0;
		data->action = action;
	}

	return data;
}

/**
* 结构体：PER_SESSION_CONTEXT
* 记录单个套接字的数据，包括了套接字的变量及其对应的客户端地址
*/
typedef struct
{
	SOCKET				socket;
	SOCKADDR_STORAGE	ClientAddr;
} PER_SESSION_CONTEXT;

// 全局变量
#define HOST "0.0.0.0"
#define PORT "7000"

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortFd);
// DWORD WINAPI ServerSendThread(LPVOID lpParam);

int main(int argc, char **argv) {

	// 请求 2.2 版本的 WinSock 库
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup: %s\n", GetLastError());
		return 1;
	}

	// 检查是否获得了所需版本的套接字库
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		fprintf(stderr, "WSAVersion\n");
		return 2;
	}

	// 创建进程全局的 IOCP FD
	HANDLE CompletionPortFD;
	if ((CompletionPortFD = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
	{
		WSACleanup();
		fprintf(stderr, "CreateIoCompletionPort: %s\n", GetLastError());
		return 3;
	}

	// 创建工作线程
	for (int i = 0; i < 1; ++i)
	{
		HANDLE ThreadHandle;
		if ((ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, CompletionPortFD, 0, NULL)) == NULL)
		{
			WSACleanup();
			fprintf(stderr, "CreateThread: %s\n", GetLastError());
			return 4;
		}
		CloseHandle(ThreadHandle);
	}

	// 建立 FD 并侦听
	SOCKET serverSocket;// = socket(AF_INET, SOCK_STREAM, 0);

	struct addrinfo hints, *ai, *p;
	int result, yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((result = getaddrinfo(HOST, PORT, &hints, &ai)) != 0)
	{
		WSACleanup();
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		return 9;
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (serverSocket < 0) continue;

		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int));

		if (bind(serverSocket, p->ai_addr, p->ai_addrlen) < 0)
		{
			closesocket(serverSocket);
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		WSACleanup();
		fprintf(stderr, "failed to bind.\n");
		return 5;
	}

	freeaddrinfo(ai);

	// 侦听
	if (listen(serverSocket, 10) == SOCKET_ERROR)
	{
		WSACleanup();
		fprintf(stderr, "listen: %s\n", GetLastError());
		return 6;
	}

	PER_SESSION_CONTEXT	*PerHandleData = NULL;
	SOCKADDR_IN			saClient;
	int					saLen = sizeof(saClient);
	SOCKET				clientFd;

	for (;;)
	{
		// 接收到客户端连接
		if ((clientFd = accept(serverSocket, (SOCKADDR*)&saClient, &saLen)) == SOCKET_ERROR)
		{
			WSACleanup();
			fprintf(stderr, "accept: %s\n", GetLastError());
			return 7;
		}

		// 关联到 IOCP
		PerHandleData = (PER_SESSION_CONTEXT*)GlobalAlloc(GPTR, sizeof(PER_SESSION_CONTEXT));
		PerHandleData->socket = clientFd;
		memcpy(&PerHandleData->ClientAddr, &saClient, saLen);

		if (CreateIoCompletionPort((HANDLE)PerHandleData->socket, CompletionPortFD, (DWORD)PerHandleData, 0) == NULL)
		{
			WSACleanup();
			fprintf(stderr, "CreateIoCompletionPort with client: %s\n", GetLastError());
			return 8;
		}

		// 使用 Overlapped I/O 进行第一次接收调用
		PER_IO_CONTEXT *PerIoData = NULL;
		PerIoData = InitAnPerIoData(WsaRead);

		DWORD receiveBytes;
		DWORD flags = 0;
		WSARecv(PerHandleData->socket, &PerIoData->databuff, 1, &receiveBytes, &flags, &PerIoData->overlapped, NULL);
	}

	WSACleanup();

	return 0;
}

/**
* 定义工作线程函数
*/
DWORD WINAPI ServerWorkThread(LPVOID iocpFd)
{
	DWORD				bytesTransferred;
	OVERLAPPED			*lpOverlapped;
	PER_SESSION_CONTEXT	*perHandleData = NULL;
	PER_IO_CONTEXT		*perIoData = NULL;
	DWORD				bytesReceived;
	DWORD				flags = 0;
	BOOL				result = FALSE;

	for (;;)
	{
		if (GetQueuedCompletionStatus((HANDLE)iocpFd, &bytesTransferred, (PULONG_PTR)&perHandleData, (LPOVERLAPPED*)&lpOverlapped, INFINITE) == FALSE)
		{
			fprintf(stderr, "GetQueuedCompletionStatus: %s\n", GetLastError());
			return 1;
		}
		perIoData = (PER_IO_CONTEXT*)CONTAINING_RECORD(lpOverlapped, PER_IO_CONTEXT, overlapped);

		// 检查是否有错误发生
		if (bytesTransferred == 0)
		{
			fprintf(stdout, "Client #%d was quit.\n", perHandleData->socket);
			closesocket(perHandleData->socket);
			GlobalFree(perHandleData);
			GlobalFree(perIoData);
			continue;
		}

		switch (perIoData->action)
		{
		case WsaRead:

			// 因为使用单工作线程，所以下面两句不需要互斥保护

			// 将接收缓冲区进行末尾截断
			perIoData->databuff.buf[bytesTransferred] = '\0';
			perIoData->bytesLength = bytesTransferred;

			// 开始接收来自客户端的数据
			fprintf(stdout, "An client says %d chars: %s\n", bytesTransferred, perIoData->databuff.buf);

			// 将相同的数据做 echo 回显
			perIoData->databuff.len = bytesTransferred;
			perIoData->action = WsaWrite;

			ZeroMemory(&perIoData->overlapped, sizeof(OVERLAPPED));
			if (WSASend(perHandleData->socket, &perIoData->databuff, 1, &perIoData->bytesLength, flags, &perIoData->overlapped, NULL) != 0
				&& WSAGetLastError() != WSA_IO_PENDING)
			{
				fprintf(stderr, "WSASend: %d\n", WSAGetLastError());
			}
			
			break;

		case WsaWrite:

			fprintf(stdout, "Tell the client %d chars: %s\n", bytesTransferred, perIoData->databuff.buf);

			// 准备下一轮数据读取
			perIoData->databuff.len = 1024;
			perIoData->action = WsaRead;
			ZeroMemory(&perIoData->overlapped, sizeof(OVERLAPPED));
			if (WSARecv(perHandleData->socket, &perIoData->databuff, 1, &bytesReceived, &flags, &perIoData->overlapped, NULL) != 0
				&& WSAGetLastError() != WSA_IO_PENDING)
			{
				fprintf(stderr, "WSARecv: %d\n", WSAGetLastError());
			}

			break;

		default:
			break;
		}

	}

	return 0;
}