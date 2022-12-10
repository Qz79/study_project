#pragma once

#include<string>
#include < vector> 
#include<list>
#include<map>

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
		hEvent = pack.hEvent;
	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize,HANDLE hEvent) {
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
		this->hEvent = hEvent;
	}
	//BYTE = unsigned char ,size_t =unsigned int ;
	//nSize 作为传入传出参数，传入时为data的长度，传出是用掉了多少
	CPacket(const BYTE* pData, size_t& nSize):hEvent(INVALID_HANDLE_VALUE)
	{
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不全，或包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize) {//包未完全接收到，就返回，解析失败
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
		//TRACE("sSum:%d\r\n", sSum);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		//TRACE("sum:%d\r\n",sum);
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
			hEvent = pack.hEvent;
		}
		return *this;
	}
	int Size() {
		//求包的长度
		return nLength + 6;
	}
	const char* Data(std::string& strOut)const {
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
	//std::string strOut;  //被弃用，由于需要从控制层过桥发送给视图层，
	//所以不可以改变包对象，只能使用包数据
	HANDLE hEvent;
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
		//静态函数没有this指针，所以无法访问成员变量
		if (m_istance == NULL) {
			m_istance = new CClinetSocket();
		}
		return m_istance;
	}
	bool InitSocket() {
		if (m_clisock != INVALID_SOCKET)closesocket(m_clisock);
		m_clisock = socket(PF_INET, SOCK_STREAM, 0);//初始化套接字
		TRACE("Create clisock:%d\r\n", m_clisock);
		if (m_clisock == -1)return false;
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));//对结构体变量初始化
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr =htonl(m_nIP);
		serv_addr.sin_port = htons(m_nPort);
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
	void UpdateAddr(int nIP, int nPort) {
		m_nIP = nIP;
		m_nPort = nPort;
	}
#define BUFFER_SIZE 4096000
	int  DealCommand() {
		/*
		对于缓冲区的操作还是需要理清逻辑的，这里除了index标记以外，还有就是定义的缓冲区中数据的问题
		*/	
		if (m_clisock == -1)return -1;
		char* buffer = m_buffer.data();
		//memset(buffer, 0, BUFFER_SIZE);这里将缓冲区重置为0，出现问题之一
		static size_t index = 0;//标记缓冲区buffer的下标 由于读取未必是全部读取所以index也会重置为0，这是问题之二
		while (true) {
			//len 接收到数据的大小
			size_t len = recv(m_clisock, buffer + index, BUFFER_SIZE - index, 0);
			//TRACE("len:%d\r\n", len);
			if (((int)len <= 0)&&((int)index<=0)) {//当缓冲区依旧有数据，只有index剩余长度读取完毕才会全部去读，问题之三
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
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
	bool Send(const CPacket& pack) {
		if (m_clisock == -1)return false;
		std::string strOut = "";
		pack.Data(strOut);
		TRACE("Clinet send Data:%s\r\n", pack.strData);
		bool ret = send(m_clisock, strOut.c_str(), strOut.size(), 0);
		return ret;
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
	static void threadEntryFunc(void* arg);
	void threadFunc();
	CClinetSocket(const CClinetSocket& ss) {
		m_clisock = ss.m_clisock;
		m_nIP = ss.m_nIP;
		m_nPort = ss.m_nPort;
	}
	CClinetSocket& operator=(const CClinetSocket& ss) {
		m_clisock = ss.m_clisock;
		m_nIP = ss.m_nIP;
		m_nPort = ss.m_nPort;
	}
	CClinetSocket():m_nIP(INADDR_ANY),m_nPort(0) {
		m_clisock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始套接字环境,请检网络设置"), _T("网络环境初始化失败"), MB_OK | MB_ICONERROR);
			exit(0);
		}	
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
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
	std::list<CPacket> lstSend;
	std::map<HANDLE, std::list<CPacket>> m_mapAck;
	std::vector<char> m_buffer;
	static CClinetSocket* m_istance;
	static Helper m_helper;
	SOCKET m_clisock;
	CPacket m_packet;
	int m_nIP;
	int m_nPort;
};

