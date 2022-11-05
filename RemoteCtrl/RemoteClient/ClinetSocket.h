#pragma once

#include<string>
#include < vector> 

#pragma pack(push) //�÷�װ�����
#pragma pack(1)
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}

		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	//BYTE = unsigned char ,size_t =unsigned int ;
	//nSize ��Ϊ���봫������������ʱΪdata�ĳ��ȣ��������õ��˶���
	CPacket(const BYTE* pData, size_t& nSize)
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
		if (nLength + i > nSize) {
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	~CPacket() {}
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
	int Size() {
		//����ĳ���
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
public:
	// WORD = unsigned short��2���ֽڣ�  DWORD = unsigend long��4���ֽڣ�
	WORD sHead;       //���ݰ���ͷ �̶�FEFF
	DWORD nLength;    //���ݳ��ȣ��ӿ������ʼ����У�����
	WORD sCmd;        //��������
	std::string strData; // ������ 
	WORD sSum;           // ��У�� ���˰�ͷ�ͳ�����������ݼ�����
	std::string strOut;  /*����strDataҲ��һ������������������ݵ�ʱ���ǵ�ַ��
						 ������Ҫ����������һ���Լ���������ȥ�洢������������*/
};
#pragma pack(pop)
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		pointXY.x = 0;
		pointXY.y = 0;
	}
	WORD nAction;// �ƶ���˫��������
	WORD nButton;//������н����Ҽ�
	POINT pointXY;//����
}MOUSEEV, * PMOUSEEV;
typedef struct file_info {
	file_info() {
		Isinvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(FileName, 0, sizeof(FileName));
	}
	BOOL Isinvalid;
	BOOL IsDirectory;
	BOOL HasNext;
	char FileName[256];
}FILEINFO, * PFILEINFO;
std::string GetErrorInfo(int wsaErrCode);
class CClinetSocket
{
public:
	static CClinetSocket* getInstance() {
		//��̬����û��thisָ�룬�����޷����ʳ�Ա����
		if (m_istance == NULL) {
			m_istance = new CClinetSocket();
		}
		return m_istance;
	}
	bool InitSocket(int AddressIP,int nPort) {
		if (m_clisock != INVALID_SOCKET)closesocket(m_clisock);
		m_clisock = socket(PF_INET, SOCK_STREAM, 0);//��ʼ���׽���
		TRACE("Create clisock:%d\r\n", m_clisock);
		if (m_clisock == -1)return false;
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));//�Խṹ�������ʼ��
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr =htonl(AddressIP);
		serv_addr.sin_port = htons(nPort);
		int ret = connect(m_clisock, (sockaddr*)&serv_addr, sizeof(serv_addr));
		if (serv_addr.sin_addr.s_addr == INVALID_SOCKET) {
			AfxMessageBox("ָ��IP������");
			return false;
		}
		if (ret == -1) {
			AfxMessageBox("����ʧ�ܣ�");
			TRACE("ErrorId:%d ,ErrorMessage:%s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}
	
#define BUFFER_SIZE 4096
	int  DealCommand() {
		//��������
		
		TRACE("client deal is start\r\n");
		if (m_clisock == -1)return -1;
		char* buffer = m_buffer.data();
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;//��ǻ�����buffer���±�
		while (true) {
			//len ���յ����ݵĴ�С
			size_t len = recv(m_clisock, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			len = index;
			//TODO:��������
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	bool Send(const char* pData, int nSize) {
		if (m_clisock == -1)return false;
		return send(m_clisock, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_clisock == -1)return false;
		TRACE("Clinet send Data:%s\r\n", pack.strData);
		return send(m_clisock, pack.Data(), pack.Size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {
			strPath = m_packet.strData;
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
	CClinetSocket(const CClinetSocket& ss) {
		m_clisock = ss.m_clisock;
	}
	CClinetSocket& operator=(const CClinetSocket& ss) {
		m_clisock = ss.m_clisock;
	}
	CClinetSocket() {
		m_clisock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ�׽��ֻ���,�����������"), _T("���绷����ʼ��ʧ��"), MB_OK | MB_ICONERROR);
			exit(0);
		}	
		m_buffer.resize(BUFFER_SIZE);
	}
	~CClinetSocket() {
		closesocket(m_clisock);
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
			CClinetSocket* temp = m_istance;
			m_istance = NULL;
			delete temp;
		}
	}
	class Helper {
	public:
		Helper() {
			CClinetSocket::getInstance();
		}
		~Helper() {
			CClinetSocket::releaseInstance();
		}

	};
private:
	std::vector<char> m_buffer;
	static CClinetSocket* m_istance;
	static Helper m_helper;
	SOCKET m_clisock;
	CPacket m_packet;
};
