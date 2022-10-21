#pragma once
#include "pch.h"
#include "framework.h"
class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		//��̬����û��thisָ�룬�����޷����ʳ�Ա����
		if (m_istance == NULL) {
			m_istance = new CServerSocket();
		}
		return m_istance;
	}
	bool InitSocket() {
		if (m_servsock == -1)return false;
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));//�Խṹ�������ʼ��
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(9527);
		if (bind(m_servsock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
			return false;
		if (listen(m_servsock, 1) == -1)return false;
		return true;
	}
	bool AcceptClient() {
		sockaddr_in cli_addr;
		memset(&cli_addr, 0, sizeof(cli_addr));
		int length = sizeof(cli_addr);
		m_clisock = accept(m_clisock, (sockaddr*)&cli_addr, &length);
		if (m_clisock == -1)return false;
		return true;
		//closesocket(cli_sock);//TODO:���ӿͻ��˵��׽���ʲôʱ���ͷţ�
	}
	int  DealCommand() {
		//��������
		if (m_clisock == -1)return false;
		char buffer[1024] = "";
		while (true) {
			int len=recv(m_clisock, buffer, sizeof(buffer), 0);
			if (len <= 0) {
				return -1;
			}
			//TODO:��������
		}
	}
	bool Send(const char* pData, int nSize) {
		if (m_clisock == -1)return false;
		return send(m_clisock, pData, nSize, 0)>0;
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
			MessageBox(NULL, _T("�޷���ʼ�׽��ֻ���,�����������"), _T("���绷����ʼ��ʧ��"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_servsock = socket(PF_INET, SOCK_STREAM, 0);//��ʼ���׽���
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
	static CServerSocket* m_istance;
	static Helper m_helper;
	SOCKET m_servsock;
	SOCKET m_clisock;
};

