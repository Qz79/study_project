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
class EBuffer :public std::string
{
public:
	EBuffer(const char* str) {
		resize(strlen(str));
		memcpy((void*)c_str(), str, size());
	}
	EBuffer(size_t size = 0) :std::string() {
		if (size > 0) {
			resize(size);
			memset(*this, 0, this->size());
		}
	}
	EBuffer(void* buffer, size_t size) :std::string() {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
	~EBuffer() {
		std::string::~basic_string();
	}
	operator char* () const { return (char*)c_str(); }
	operator const char* () const { return c_str(); }
	operator BYTE* () const { return (BYTE*)c_str(); }
	operator void* () const { return (void*)c_str(); }
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
};
class NetAddress {
public:
	NetAddress() {
		m_port = -1;
		memset(&m_addr, 0, sizeof(m_addr));
		m_addr.sin_family = AF_INET;
	}
	NetAddress(const NetAddress& addr) {
		m_ip = addr.m_ip;
		m_port = addr.m_port;
		m_addr = addr.m_addr;
	}
	void Update(const std::string& ip, short port) {
		m_ip = ip;
		m_port = port;
	}
	int Size()const { 
		return sizeof(sockaddr_in);
	}
	NetAddress& operator=(const NetAddress& addr) {
		if (this != &addr) {
			m_ip = addr.m_ip;
			m_port = addr.m_port;
			m_addr = addr.m_addr;
		}
		return *this;
	}
	operator sockaddr* ()const {
		return (sockaddr*)&m_addr;
	}
	operator sockaddr* () {
		return (sockaddr*)&m_addr;
	}
	operator sockaddr_in* () {
		return &m_addr;
	}
	~NetAddress() {}
private:
	std::string m_ip;
	short m_port;
	sockaddr_in m_addr;
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
	ZSocket(bool isTcp = true) :m_socket(new CServerSock(isTcp))
	, m_istcp(isTcp){}
	~ZSocket() {
		m_socket.reset();
	}
	ZSocket(const ZSocket& sock) :
		m_socket(sock.m_socket), m_istcp(sock.m_istcp) {}
	ZSocket(SOCKET sock, bool isTcp) :m_socket(new CServerSock(isTcp))
		, m_istcp(isTcp) {}
	int Bind(const 	NetAddress& addr) {
		if (m_socket == nullptr) {
			m_socket.reset(new CServerSock(m_istcp));
		}
		return bind(*m_socket, addr, addr.Size());
	}
	int Listen(int backLog = 5) {
		return listen(*m_socket, backLog);
	}
	int Connect(const NetAddress& addr) {
		return connect(*m_socket, addr, addr.Size());
	}
	void Close() {
		m_socket.reset();
	}
	int Recv(const EBuffer& buffer) {
		return recv(*m_socket, buffer, buffer.size(), 0);
	}
	int Send(const EBuffer& buffer) {
		//TODO:待优化
		return send(*m_socket,buffer, buffer.size(), 0);
	}
	ZSocket Accpect(const NetAddress& addr) {
		int len = addr.Size();
		SOCKET s = accept(*m_socket, addr, &len);
		return s;
	}
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
	bool m_istcp;
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

