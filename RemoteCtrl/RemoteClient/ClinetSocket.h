#pragma once

#include<string>
#include < vector> 

#pragma pack(push) //让封装类对齐
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
	//nSize 作为传入传出参数，传入时为data的长度，传出是用掉了多少
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
		//求包的长度
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
	// WORD = unsigned short（2个字节）  DWORD = unsigend long（4个字节）
	WORD sHead;       //数据包包头 固定FEFF
	DWORD nLength;    //数据长度，从控制命令开始到和校验结束
	WORD sCmd;        //控制命令
	std::string strData; // 包数据 
	WORD sSum;           // 和校验 除了包头和长度以外的数据加起来
	std::string strOut;  /*由于strData也是一个对象，所以在输出数据的时候是地址，
						 所以需要在类中声明一个自己缓冲区，去存储整个包的数据*/
};
#pragma pack(pop)
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		pointXY.x = 0;
		pointXY.y = 0;
	}
	WORD nAction;// 移动、双击、单击
	WORD nButton;//左键、中建、右键
	POINT pointXY;//坐标
}MOUSEEV, * PMOUSEEV;
std::string GetErrorInfo(int wsaErrCode);
class CClinetSocket
{
public:
	static CClinetSocket* getInstance() {
		//静态函数没有this指针，所以无法访问成员变量
		if (m_istance == NULL) {
			m_istance = new CClinetSocket();
		}
		return m_istance;
	}
	bool InitSocket(int AddressIP,int nPort) {
		if (m_clisock != INVALID_SOCKET)closesocket(m_clisock);
		m_clisock = socket(PF_INET, SOCK_STREAM, 0);//初始化套接字
		TRACE("Create clisock:%d\r\n", m_clisock);
		if (m_clisock == -1)return false;
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));//对结构体变量初始化
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr =htonl(AddressIP);
		serv_addr.sin_port = htons(nPort);
		int ret = connect(m_clisock, (sockaddr*)&serv_addr, sizeof(serv_addr));
		if (serv_addr.sin_addr.s_addr == INVALID_SOCKET) {
			AfxMessageBox("指定IP不存在");
			return false;
		}
		if (ret == -1) {
			AfxMessageBox("链接失败！");
			TRACE("ErrorId:%d ,ErrorMessage:%s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}
	
#define BUFFER_SIZE 4096
	int  DealCommand() {
		//处理链接
		
		TRACE("client deal is start\r\n");
		if (m_clisock == -1)return -1;
		char* buffer = m_buffer.data();
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;//标记缓冲区buffer的下标
		while (true) {
			//len 接收到数据的大小
			size_t len = recv(m_clisock, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			len = index;
			//TODO:处理命令
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
			MessageBox(NULL, _T("无法初始套接字环境,请检网络设置"), _T("网络环境初始化失败"), MB_OK | MB_ICONERROR);
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

