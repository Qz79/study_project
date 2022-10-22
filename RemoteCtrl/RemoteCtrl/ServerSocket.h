#pragma once
#include "pch.h"
#include "framework.h"
#include<string>
class CPacket {
public:
	CPacket():sHead(0),nLength(0),sCmd(0),sSum(0){}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	//BYTE = unsigned char ,size_t =unsigned int ;
	//nSize ��Ϊ���봫������������ʱΪdata�ĳ��ȣ��������õ��˶���
	CPacket(const BYTE* pData,size_t& nSize) 
	{
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(WORD*)(pData + i);
		i += 4;
		if (nLength+i > nSize) {
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength-4;
		}
		sSum= *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[i]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	~CPacket(){}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
public:
	// WORD = unsigned short��2���ֽڣ�  DWORD = unsigend long��4���ֽڣ�
	WORD sHead;       //���ݰ���ͷ �̶�FEFF
	DWORD nLength;    //���ݳ��ȣ��ӿ������ʼ����У�����
	WORD sCmd;        //��������
	std::string strData; // ������
	WORD sSum;           // ��У��
};
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
#define BUFFER_SIZE 4096
	int  DealCommand() {
		//��������
		if (m_clisock == -1)return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;//��ǻ�����buffer���±�
		while (true) {
			//len ���յ����ݵĴ�С
			size_t len=recv(m_clisock, buffer+index, BUFFER_SIZE -index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			len = index;
			//TODO:��������
			m_packet = CPacket ((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1
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
	CPacket m_packet;
};

