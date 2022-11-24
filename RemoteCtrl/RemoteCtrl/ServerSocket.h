#pragma once
#include "pch.h"
#include "framework.h"
#include<string>
#include"Packet.h"
typedef void(*SOCKET_CALLBACK)(void*,int, std::list<CPacket>&, CPacket&);
class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		//静态函数没有this指针，所以无法访问成员变量
		if (m_istance == NULL) {
			m_istance = new CServerSocket();
		}
		return m_istance;
	}
	bool InitSocket(short port) {
		if (m_servsock == -1)return false;
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));//对结构体变量初始化
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);
		if (bind(m_servsock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
			return false;
		if (listen(m_servsock, 1) == -1)return false;
		return true;
	}
	int Run(SOCKET_CALLBACK callback,void* arg,short port=9527) {
		if (InitSocket(port) == false)return-1;
		std::list<CPacket> lstPackets;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			if (ret > 0) {
				m_callback(m_arg, ret, lstPackets,m_packet);
				while (lstPackets.size() > 0) {
					Send(lstPackets.front());
					lstPackets.pop_front();
				}
			}
			CloseCliSocket();
		}
	}
	bool AcceptClient() {
		sockaddr_in cli_addr;
		memset(&cli_addr, 0, sizeof(cli_addr));
		int length = sizeof(cli_addr);
		m_clisock = accept(m_servsock, (sockaddr*)&cli_addr, &length);
		TRACE("sever->m_clisock:%d\r\n", m_clisock);
		if (m_clisock == -1)return false;
		return true;
		//closesocket(cli_sock);//TODO:链接客户端的套接字什么时候释放？
	}
#define BUFFER_SIZE 409600
	int  DealCommand() {
		//处理链接
		TRACE("sever deal is start\r\n");
		if (m_clisock == -1)return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;//标记缓冲区buffer的下标
		while (true) {
			//len 接收到数据的大小
			size_t len=recv(m_clisock, buffer+index, BUFFER_SIZE -index, 0);
			if (len <= 0) {
				return -1;
				delete[] buffer;
			}
			index += len;
			len = index;
			//TODO:处理命令
			m_packet = CPacket ((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
				delete[] buffer;
			}
		}
		delete[] buffer;
		return -1;
	}
	bool Send(const char* pData, int nSize) {
		if (m_clisock == -1)return false;
		return send(m_clisock, pData, nSize, 0)>0;
	}
	bool Send(CPacket& pack) {
		if (m_clisock == -1)return false;
		//Dump((BYTE*)pack.Data(), pack.Size());
		return send(m_clisock, pack.Data(), pack.Size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2)&&(m_packet.sCmd<=4)||(m_packet.sCmd==9)) {
			strPath = m_packet.strData;
			TRACE("GetStrPath=%s\r\n", m_packet.strData);
			return true;
		}
		return false;
	}
	bool MoueEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(mouse));
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseCliSocket() {
		closesocket(m_clisock);
		m_clisock = INVALID_SOCKET;
	}
private:
	CServerSocket(const CServerSocket& ss){
		m_servsock = ss.m_servsock;
		m_clisock = ss.m_clisock;
	}
	CServerSocket& operator=(const CServerSocket& ss){
		m_servsock = ss.m_servsock;
		m_clisock = ss.m_clisock;
	}
	CServerSocket(){
		m_servsock = INVALID_SOCKET;
		m_clisock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始套接字环境,请检网络设置"), _T("网络环境初始化失败"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_servsock = socket(PF_INET, SOCK_STREAM, 0);//初始化套接字
	}
	~CServerSocket(){
		closesocket(m_servsock);
		WSACleanup();
	}
	BOOL InitSockEnv() {
		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(1, 1), &wsadata) != 0) {
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance() {
		if (m_istance != NULL) {
			CServerSocket* temp = m_istance;
			m_istance = NULL;
			delete temp;
		}
	}
	class Helper {
	public:
		Helper(){
			CServerSocket::getInstance();
		}
		~Helper() {
			CServerSocket::releaseInstance();
		}
	
	};
private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	static CServerSocket* m_istance;
	static Helper m_helper;
	SOCKET m_servsock;
	SOCKET m_clisock;
	CPacket m_packet;
};

