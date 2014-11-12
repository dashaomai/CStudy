#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")   // WinSock2 静态链接库
#pragma comment(lib, "Kernel32.lib") // IOCP 依赖的内核库

/**
 * 结构体：PER_IO_DATA
 * Overlapped I/O 需要使用的结构体，临时记录 IO 数据
 */
#define DataBuffSize 2048 // 2 * 1024;
typedef struct
{
	OVERLAPPED	overlapped;
	WSABUF		databuff;
	char		buffer[DataBuffSize];
	int			BufferLen;
	int			operationType;
} PER_IO_DATA;

/**
 * 结构体：PER_HANDLE_DATA
 * 记录单个套接字的数据，包括了套接字的变量及其对应的客户端地址
 */
typedef struct
{
	SOCKET				socket;
	SOCKADDR_STORAGE	ClientAddr;
} PER_HANDLE_DATA;

#define ERR_EXIT(message, code) fprintf(stderr, messgae);WSACleanup();return code

// 全局变量
const u_int PORT = 7000;

HANDLE hMutex;

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortFd);
// DWORD WINAPI ServerSendThread(LPVOID lpParam);

int main(int argc, char **argv) {
	hMutex = CreateMutex(NULL, FALSE, NULL);

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

	// 建立 FD
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	// 绑定端口
	SOCKADDR_IN serverAddr;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);	// PORT
	if (bind(serverSocket, (SOCKADDR *)&serverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		WSACleanup();
		fprintf(stderr, "bind: %s\n", GetLastError());
		return 5;
	}

	// 侦听
	if (listen(serverSocket, 10) == SOCKET_ERROR)
	{
		WSACleanup();
		fprintf(stderr, "listen: %s\n", GetLastError());
		return 6;
	}

	PER_HANDLE_DATA	*PerHandleData = NULL;
	SOCKADDR_IN		saClient;
	int				saLen = sizeof(saClient);
	SOCKET			clientFd;

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
		PerHandleData = (PER_HANDLE_DATA*)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
		PerHandleData->socket = clientFd;
		memcpy(&PerHandleData->ClientAddr, &saClient, saLen);

		if (CreateIoCompletionPort((HANDLE)PerHandleData->socket, CompletionPortFD, (DWORD)PerHandleData, 0) == NULL)
		{
			WSACleanup();
			fprintf(stderr, "CreateIoCompletionPort with client: %s\n", GetLastError());
			return 8;
		}

		// 使用 Overlapped I/O 进行第一次接收调用
		PER_IO_DATA *PerIoData = NULL;
		PerIoData = (PER_IO_DATA*)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;		// read

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
DWORD WINAPI ServerWorkThread(LPVOID IOCPFD)
{
	HANDLE			CompletionPortFd = (HANDLE)IOCPFD;
	DWORD			bytesTransferred;
	OVERLAPPED		*lpOverlapped;
	PER_HANDLE_DATA *perHandleData = NULL;
	PER_IO_DATA		*perIoData = NULL;
	DWORD			bytesReceived;
	DWORD			flags = 0;
	BOOL			result = FALSE;

	for (;;)
	{
		if (GetQueuedCompletionStatus(CompletionPortFd, &bytesTransferred, &perHandleData, (OVERLAPPED**)&lpOverlapped, INFINITE) == NULL)
		{
			fprintf(stderr, "GetQueuedCompletionStatus: %s\n", GetLastError());
			return 1;
		}
		perIoData = (PER_IO_DATA*)CONTAINING_RECORD(lpOverlapped, PER_IO_DATA, overlapped);

		// 检查是否有错误发生
		if (bytesTransferred == 0)
		{
			closesocket(perHandleData->socket);
			GlobalFree(perHandleData);
			GlobalFree(perIoData);
			continue;
		}

		// 开始接收来自客户端的数据
		// 在这里运用互斥来独立访问 PerIoData 内部的缓冲区，难道一个 IO 上的多次异步数据流可能被其它线程处理到吗？
		// 如果设置只有一个工作线程，是不是就可以取消该互斥了？
		WaitForSingleObject(hMutex, INFINITE);
		fprintf(stdout, "An client says: %s\n", perIoData->databuff.buf);
		ReleaseMutex(hMutex);

		// 准备下一轮数据读取
		ZeroMemory(&perIoData->overlapped, sizeof(OVERLAPPED));
		perIoData->databuff.len = 1024;
		perIoData->databuff.buf = perIoData->buffer;
		perIoData->operationType = 0;		// read
		WSARecv(perHandleData->socket, &perIoData->databuff, 1, &bytesReceived, &flags, &perIoData->overlapped, NULL);
	}

	return 0;
}