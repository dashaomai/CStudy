#include <stdio.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")   // WinSock2 ��̬���ӿ�
#pragma comment(lib, "Kernel32.lib") // IOCP �������ں˿�

typedef enum WsaAction
{
	WsaAccept,
	WsaWrite,
	WsaRead
} WsaAction;

/**
* �ṹ�壺PER_IO_CONTEXT
* Overlapped I/O ��Ҫʹ�õĽṹ�壬��ʱ��¼ IO ����
*/
#define DataBuffSize 2048 // 2 * 1024 ��ª��˫�ֽڼ��ݷ���
typedef struct
{
	OVERLAPPED	overlapped;
	WSABUF		databuff;
	DWORD		bytesLength;
	WsaAction	action;
} PER_IO_CONTEXT;

/**
 * ��ʼ��������һ�� PER_IO_DATA
 */
PER_IO_CONTEXT* InitAnPerIoData(WsaAction action)
{
	PER_IO_CONTEXT* data = NULL;
	// ����Ŀռ��С�� PER_IO_CONTEXT �ṹ�屾������⣬����������� DataBuffSize ��С���ֽ����飬��Ϊ data->databuff.buf ����
	// TODO: ������������ڴ�ռ䣬�ᱻ�Զ��������������������ص���
	data = (PER_IO_CONTEXT*)GlobalAlloc(GPTR, sizeof(PER_IO_CONTEXT) + DataBuffSize * sizeof(char));

	if (data != NULL)
	{
		data->databuff.len = 1024;
		data->databuff.buf = (char*)(&data->databuff + sizeof(data->databuff.len));		// TODO: ����ֱ��ָ��ָ���Ƿ���Σ���ԣ�
		data->bytesLength = 0;
		data->action = action;
	}

	return data;
}

/**
* �ṹ�壺PER_SESSION_CONTEXT
* ��¼�����׽��ֵ����ݣ��������׽��ֵı��������Ӧ�Ŀͻ��˵�ַ
*/
typedef struct
{
	SOCKET				socket;
	SOCKADDR_STORAGE	ClientAddr;
} PER_SESSION_CONTEXT;

// ȫ�ֱ���
#define HOST "0.0.0.0"
#define PORT "7000"

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortFd);
// DWORD WINAPI ServerSendThread(LPVOID lpParam);

int main(int argc, char **argv) {

	// ���� 2.2 �汾�� WinSock ��
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup: %s\n", GetLastError());
		return 1;
	}

	// ����Ƿ���������汾���׽��ֿ�
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		fprintf(stderr, "WSAVersion\n");
		return 2;
	}

	// ��������ȫ�ֵ� IOCP FD
	HANDLE CompletionPortFD;
	if ((CompletionPortFD = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
	{
		WSACleanup();
		fprintf(stderr, "CreateIoCompletionPort: %s\n", GetLastError());
		return 3;
	}

	// ���������߳�
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

	// ���� FD ������
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

	// ����
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
		// ���յ��ͻ�������
		if ((clientFd = accept(serverSocket, (SOCKADDR*)&saClient, &saLen)) == SOCKET_ERROR)
		{
			WSACleanup();
			fprintf(stderr, "accept: %s\n", GetLastError());
			return 7;
		}

		// ������ IOCP
		PerHandleData = (PER_SESSION_CONTEXT*)GlobalAlloc(GPTR, sizeof(PER_SESSION_CONTEXT));
		PerHandleData->socket = clientFd;
		memcpy(&PerHandleData->ClientAddr, &saClient, saLen);

		if (CreateIoCompletionPort((HANDLE)PerHandleData->socket, CompletionPortFD, (DWORD)PerHandleData, 0) == NULL)
		{
			WSACleanup();
			fprintf(stderr, "CreateIoCompletionPort with client: %s\n", GetLastError());
			return 8;
		}

		// ʹ�� Overlapped I/O ���е�һ�ν��յ���
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
* ���幤���̺߳���
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

		// ����Ƿ��д�����
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

			// ��Ϊʹ�õ������̣߳������������䲻��Ҫ���Ᵽ��

			// �����ջ���������ĩβ�ض�
			perIoData->databuff.buf[bytesTransferred] = '\0';
			perIoData->bytesLength = bytesTransferred;

			// ��ʼ�������Կͻ��˵�����
			fprintf(stdout, "An client says %d chars: %s\n", bytesTransferred, perIoData->databuff.buf);

			// ����ͬ�������� echo ����
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

			// ׼����һ�����ݶ�ȡ
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