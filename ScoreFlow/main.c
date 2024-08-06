#include "stdafx.h"

#define SERVER_PORT "9999"
#define THREAD_COUNT 16

DWORD WINAPI HttpWorkThread(LPVOID lParam) {
	SOCKET listenSocket = *(SOCKET*)lParam;

	while (1) {
		SOCKET clientSocket;
		SOCKADDR_IN clientAddr;
		int clientAddrSize = sizeof(clientAddr);
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
		if (clientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			continue;
		}

		// printf("New client connected: %d\n", clientSocket);

		Connector* con = (Connector*)malloc(sizeof(Connector));
		con->socket = clientSocket;
		int recvResult = HandleTcpRecv(con);

		if (recvResult <= 0) {
			// printf("Client disconnected: %d\n", clientSocket);
		}

		closesocket(clientSocket);
		// printf("Client disconnected: %d\n", clientSocket);
		free(con);
	}

	return 0;
}

int callback(void* data, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main() {
	SetConsoleOutputCP(CP_UTF8);  // 콘솔 출력 인코딩을 UTF-8로 설정
	//const char* d = "GET / HTTP/1.1nHost: localhost:9999";
	//int splitSize = 0;
	//char** a = Split(d, "\r\n", &splitSize);

	WSADATA wsaData;
	HANDLE  threads[THREAD_COUNT];
	DWORD   threadIds[THREAD_COUNT];
	int		iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed; %d\n", iResult);
		return 1;
	}

	struct addrinfo* result = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, SERVER_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET listenSocket = INVALID_SOCKET;
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	printf("Create Socket Complete...\n");

	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Socket Bind Complete...\n");

	freeaddrinfo(result);

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed: %ld\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Socket Listen Complete...\n");

	//u_long iMode = 1;
	//iResult = ioctlsocket(listenSocket, FIONBIO, &iMode);
	//if (iResult == SOCKET_ERROR) {
	//	printf("ioctlsocket failed: %ld\n", WSAGetLastError());
	//	closesocket(listenSocket);
	//	WSACleanup();
	//	return 1;
	//}

	//printf("Socket NonBlock Config Complete...\n");

	printf("Waiting for client connections...\n");

	printf("Work Thread Count: %d\n", THREAD_COUNT);

	printf("================ sever Start ===============\n");

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i] = CreateThread(NULL, 0, HttpWorkThread, &listenSocket, 0, &threadIds[i]);
		if (threads[i] == NULL) {
			printf("Error creating thread %d: %d\n", i, GetLastError());
			return 1;
		}
	}

	WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);

	for (int i = 0; i < THREAD_COUNT; i++) {
		CloseHandle(threads[i]);
	}

	closesocket(listenSocket);
	WSACleanup();
	
	printf("All threads have finished executing\n");
	printf("bye~\n");
	return 0;
}