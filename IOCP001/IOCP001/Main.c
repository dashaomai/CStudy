#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")   // WinSock2 ��̬���ӿ�
#pragma comment(lib, "Kernel32.lib") // IOCP �������ں˿�

/**
 * �ṹ�壺PER_IO_DATA
 * Overlapped I/O ��Ҫʹ�õĽṹ�壬��ʱ��¼ IO ����
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
 * �ṹ�壺PER_HANDLE_DATA
 * ��¼�����׽��ֵ����ݣ��������׽��ֵı��������Ӧ�Ŀͻ��˵�ַ
 */
typedef struct
{
	SOCKET				socket;
	SOCKADDR_STORAGE	ClientAddr;
} PER_HANDLE_DATA;

#define ERR_EXIT(message, code) fprintf(stderr, messgae);WSACleanup();return code

// ȫ�ֱ���
const u_int PORT = 7000;

HANDLE hMutex;

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortFd);
// DWORD WINAPI ServerSendThread(LPVOID lpParam);

int main(int argc, char **argv) {
	hMutex = CreateMutex(NULL, FALSE, NULL);

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

	// ���� FD
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	// �󶨶˿�
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

	// ����
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
		// ���յ��ͻ�������
		if ((clientFd = accept(serverSocket, (SOCKADDR*)&saClient, &saLen)) == SOCKET_ERROR)
		{
			WSACleanup();
			fprintf(stderr, "accept: %s\n", GetLastError());
			return 7;
		}

		// ������ IOCP
		PerHandleData = (PER_HANDLE_DATA*)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
		PerHandleData->socket = clientFd;
		memcpy(&PerHandleData->ClientAddr, &saClient, saLen);

		if (CreateIoCompletionPort((HANDLE)PerHandleData->socket, CompletionPortFD, (DWORD)PerHandleData, 0) == NULL)
		{
			WSACleanup();
			fprintf(stderr, "CreateIoCompletionPort with client: %s\n", GetLastError());
			return 8;
		}

		// ʹ�� Overlapped I/O ���е�һ�ν��յ���
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
 * ���幤���̺߳���
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

		// ����Ƿ��д�����
		if (bytesTransferred == 0)
		{
			closesocket(perHandleData->socket);
			GlobalFree(perHandleData);
			GlobalFree(perIoData);
			continue;
		}

		// ��ʼ�������Կͻ��˵�����
		// ���������û������������� PerIoData �ڲ��Ļ��������ѵ�һ�� IO �ϵĶ���첽���������ܱ������̴߳�����
		// �������ֻ��һ�������̣߳��ǲ��ǾͿ���ȡ���û����ˣ�
		WaitForSingleObject(hMutex, INFINITE);
		fprintf(stdout, "An client says: %s\n", perIoData->databuff.buf);
		ReleaseMutex(hMutex);

		// ׼����һ�����ݶ�ȡ
		ZeroMemory(&perIoData->overlapped, sizeof(OVERLAPPED));
		perIoData->databuff.len = 1024;
		perIoData->databuff.buf = perIoData->buffer;
		perIoData->operationType = 0;		// read
		WSARecv(perHandleData->socket, &perIoData->databuff, 1, &bytesReceived, &flags, &perIoData->overlapped, NULL);
	}

	return 0;
}