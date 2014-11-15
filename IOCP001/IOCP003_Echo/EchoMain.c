#include "EchoMain.h"

#define HOST "0.0.0.0"
#define PORT "7000"

#define WORKER_AMOUNT 1

DWORD WINAPI WorkerThread(LPVOID iocpFd);

int main(int argc, char **argv)
{
	// ���� 2.2 �汾�� WinSock ��
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup: %d\n", GetLastError());
		return 1;
	}

	// ����Ƿ���������汾�� WinSock ��
	if (HIBYTE(wsaData.wVersion) != 2 || LOBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		fprintf(stderr, "WSAVersion\n");
		return 2;
	}

	// ����ȫ�� iocpFd
	HANDLE iocpFd;
	if ((iocpFd = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
	{
		WSACleanup();
		fprintf(stderr, "CreateIoCompletionPort: %d\n", GetLastError());
		return 3;
	}

	// ���������߳�
	for (int i = 0; i < WORKER_AMOUNT; i++)
	{
		HANDLE thread;
		if ((thread = CreateThread(NULL, 0, WorkerThread, iocpFd, 0, NULL)) == NULL)
		{
			WSACleanup();
			fprintf(stderr, "CreateThread: %d\n", GetLastError());
			return 4;
		}

		CloseHandle(thread);
	}

	// ���� Server Listeners ����ʼ����
	SOCKET listener;

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
		return 5;
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		listener = WSASocket(
			p->ai_family,
			p->ai_socktype,
			p->ai_protocol,
			NULL,
			0,
			WSA_FLAG_OVERLAPPED);
		if (listener == INVALID_SOCKET) continue;

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			closesocket(listener);
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		WSACleanup();
		fprintf(stderr, "failed to bind.\n");
		return 6;
	}

	freeaddrinfo(ai);

	// ����
	if (listen(listener, 10) == SOCKET_ERROR)
	{
		WSACleanup();
		fprintf(stderr, "listen: %d\n", GetLastError());
		return 7;
	}

	LPPER_SESSION_CONTEXT	session = NULL;
	LPPER_IO_CONTEXT		io = NULL;
	SOCKADDR				clientAddr;
	int						length = sizeof(clientAddr);
	SOCKET					clientFd;
	DWORD					flags = 0;

	// Server Loop
	for (;;)
	{
		// ���յ��ͻ�������
		if ((clientFd = WSAAccept(listener, &clientAddr, &length, NULL, (DWORD_PTR)NULL)) == INVALID_SOCKET)
		{
			WSACleanup();
			fprintf(stderr, "accept: %d\n", GetLastError());
			return 8;
		}

		// ������ IOCP
		session = CreateSessionContext(clientFd, (SOCKADDR_STORAGE*)&clientAddr, SocketRead);

		if (CreateIoCompletionPort(
			(HANDLE)session->clientSocket,
			iocpFd,
			(ULONG_PTR)session,
			0) == NULL)
		{
			WSACleanup();
			fprintf(stderr, "CreateIoCompletionPort with client: %d\n", GetLastError());
			return 9;
		}

		io = &session->ioContext;

		WSARecv(
			session->clientSocket,
			&io->buffer,
			1,
			&io->buffer.len,
			&flags,
			&io->overlapped,
			NULL
			);
	}

	WSACleanup();

	return 0;
}

DWORD WINAPI WorkerThread(LPVOID globalIocpFd)
{
	HANDLE					iocpFd = (HANDLE)globalIocpFd;
	LPOVERLAPPED			overlapped;
	LPPER_SESSION_CONTEXT	session = NULL;
	LPPER_IO_CONTEXT		io = NULL;
	DWORD					bytesTransferred;
	DWORD					flags = 0;
	BOOL					result = FALSE;

	for (;;)
	{
		if (GetQueuedCompletionStatus(
			iocpFd,
			&bytesTransferred,
			(PULONG_PTR)&session,
			&overlapped,
			INFINITE) == FALSE)
		{
			fprintf(stderr, "GetQueuedCompletionStatus: %d\n", GetLastError());
			return 1;
		}

		io = &session->ioContext;

		// ����Ƿ�ر���
		if (bytesTransferred == 0)
		{
			fprintf(stdout, "Client #%d was quit.\n", session->clientSocket);
			closesocket(session->clientSocket);
			GlobalFree(session);

			io = NULL;
			session = NULL;

			continue;
		}

		// �����������ͽ��н���д����
		switch (io->action)
		{
		case SocketRead:
			io->buffer.buf[bytesTransferred] = '\0';

			// չʾ�յ��Ŀͻ�������
			fprintf(
				stdout,
				"The client #%d said (%d) chars: %s\n",
				session->clientSocket,
				bytesTransferred,
				io->bufferStub);

			// ����ͬ�������� echo ����
			io->buffer.len = bytesTransferred;
			io->action = SocketWrite;
			memset(&io->overlapped, 0, sizeof(OVERLAPPED));
			if (WSASend(
				session->clientSocket,
				&io->buffer,
				1,
				&bytesTransferred,
				flags,
				&io->overlapped,
				NULL) != 0 && WSAGetLastError() != WSA_IO_PENDING)
			{
				fprintf(stderr, "WSASend: %d\n", WSAGetLastError());
			}

			break;

		case SocketWrite:

			// չʾ�����Ŀͻ�������
			fprintf(stdout, "Tell the client #%d (%d) chars: %s\n",
				session->clientSocket, bytesTransferred, io->bufferStub);

			// ׼����һ�����ݶ�ȡ
			io->buffer.len = BUFFER_SIZE;
			io->action = SocketRead;
			memset(&io->overlapped, 0, sizeof(OVERLAPPED));
			if (WSARecv(
				session->clientSocket,
				&io->buffer,
				1,
				&bytesTransferred,
				&flags,
				&io->overlapped,
				NULL) != 0 && WSAGetLastError() != WSA_IO_PENDING)
			{
				fprintf(stderr, "WSARecv: %d\n", WSAGetLastError());
			}

			break;


		default:
			break;
		}
	}
}