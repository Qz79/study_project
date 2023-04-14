#pragma once
#include<WinSock2.h>
class CServerSock
{
public:
	CServerSock(bool isTcp=true):m_sock(INVALID_SOCKET) {
		if (isTcp) {
			m_sock = socket(PF_INET, SOCK_STREAM, 0);
		}
		else {
			m_sock = socket(PF_INET, SOCK_DGRAM, 0);
		}
	}
	void CloseSock() {
		closesocket(m_sock);
	}
	~CServerSock(){}
private:
	SOCKET m_sock;
};
class InitSocket {
public:
	InitSocket() {
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}
	~InitSocket() {
		WSACleanup();
	}
};

