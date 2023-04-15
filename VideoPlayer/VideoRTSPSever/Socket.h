#pragma once
#include<WinSock2.h>
#include<share.h>
class CServerSock
{
public:
	CServerSock(bool isTcp = true) :m_sock(INVALID_SOCKET) {
		if (isTcp) {
			m_sock = socket(PF_INET, SOCK_STREAM, 0);
		}
		else {
			m_sock = socket(PF_INET, SOCK_DGRAM, 0);
		}
	}
	void CloseSock() {
		if (m_sock != INVALID_SOCKET) {
			SOCKET temp = m_sock;
			m_sock = INVALID_SOCKET;
			closesocket(temp);
		}
	}
	~CServerSock() {
		CloseSock();
	}
	operator SOCKET() {
		return m_sock;
	}
private:
	SOCKET m_sock;
};
/*想为套接字做赋值和引用操作，但是又需要考虑关闭套接字的问题；所以在需要解决这个问题，就要引入变量；
* 方法1.引入静态成员变量：
* 静态成员变量就会引入新的问题，就是这个CServerSock类中有TCP也有UDP，这样两个不一样的协议就会共享静态成员
* 方法2.利用map映射表：
* 这个方法还是比较可行的，但是map又比较麻烦，需要.cpp文件做一个配合，这种方式可以尝试，但是在这里只想用一个.h
* 文件作为一个工具类的文件，增加可移植性，暂时不用map
* 方法3.利用std::share_ptr<>智能指针这个东西
* 它实现的是引用计数型的智能指针 ，可以被自由地拷贝和赋值，在任意的地方共享它，当没有代码使用
*（引用计数为0）它时才删除被包装的动态分配的对象。
*/
class ZSocket {
public:
	ZSocket(bool isTcp = true) :m_socket(new CServerSock(isTcp)) {};
	~ZSocket() {
		m_socket.reset();
	}
	ZSocket(const ZSocket& sock) :
		m_socket(sock.m_socket) {}
	ZSocket& operator=(const ZSocket& sock) {
		if (this != &sock) {
			m_socket = sock.m_socket;
		}
		return *this;
	}
	operator SOCKET() {
		return *m_socket;
	}
private:
	std::shared_ptr<CServerSock> m_socket;
};
class SockerIniter {
public:
	SockerIniter() {
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}
	~SockerIniter() {
		WSACleanup();
	}
};

