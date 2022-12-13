#include "pch.h"
#include "ClinetSocket.h"
CClinetSocket* CClinetSocket::m_istance = NULL;
CClinetSocket::Helper CClinetSocket::m_helper;
CClinetSocket* pclient = CClinetSocket::getInstance();
std::string GetErrorInfo(int wsaErrCode) {
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

CClinetSocket::CClinetSocket(const CClinetSocket& ss) {
	m_hThread = INVALID_HANDLE_VALUE;
	m_bAutoClose = ss.m_bAutoClose;
	m_clisock = ss.m_clisock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
	std::map<UINT, CClinetSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
	for (; it != ss.m_mapFunc.end(); it++) {
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
	}
}

CClinetSocket::CClinetSocket() :
	m_nIP(INADDR_ANY), m_nPort(0), m_clisock(INVALID_SOCKET), m_bAutoClose(true),
	m_hThread(INVALID_HANDLE_VALUE)
{
	if (InitSockEnv() == FALSE) {
		MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_eventInvoke = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClinetSocket::threadEntryFunc, this, 0, &m_nThreadID);
	if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT) {
		TRACE("网络消息处理线程启动失败了！\r\n");
	}
	CloseHandle(m_eventInvoke);
	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);
	struct {
		UINT message;
		MSGFUNC func;
	}funcs[] = {
		{WM_SEND_PACK,&CClinetSocket::SendPack},
		{0,NULL}
	};
	for (int i = 0; funcs[i].message != 0; i++) {
		if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
			TRACE("插入失败，消息值：%d 函数值:%08X 序号:%d\r\n", funcs[i].message, funcs[i].func, i);
		}
	}
}

bool CClinetSocket::SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed, WPARAM wParam)
{
	UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;
	std::string strOut;
	pack.Data(strOut);
	PACKET_DATA* pData = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam);
	bool ret = PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)pData, (LPARAM)hWnd);
	if (ret == false) {
		delete pData;
	}
	return ret;
}

unsigned CClinetSocket::threadEntryFunc(void* arg)
{
	CClinetSocket* thiz = (CClinetSocket*)arg;
	thiz->threadFunc2();
	_endthreadex(0);
	return 0;
}

void CClinetSocket::threadFunc2()
{
	SetEvent(m_eventInvoke);
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		TRACE("Get Message :%08X \r\n", msg.message);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}


//void CClinetSocket::threadFunc()
//{
//	if (InitSocket() == false)return;
//	std::string strBuffer;
//	strBuffer.resize(BUFFER_SIZE);
//	char* pBuffer = (char*)strBuffer.c_str();
//	int index = 0;
//	while (m_clisock != INVALID_SOCKET) {
//		if (lstSend.size() > 0) {
//			CPacket& head = lstSend.front();
//			if (Send(head) == false) {
//				TRACE("发送失败\r\n");
//				continue;
//			}
//			auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list < CPacket>()));
//			int length = recv(m_clisock, pBuffer, BUFFER_SIZE - index, 0);
//			if (length > 0 || index > 0) {
//				index += length;
//				size_t size = (size_t)index;
//				CPacket pack((BYTE*)pBuffer, size);	
//				if (size > 0) {
//					//TODO:通知对应的事件
//					pack.hEvent = head.hEvent;
//					pr.first->second.push_back(pack);
//					SetEvent(head.hEvent);
//					
//				}
//				continue;
//			}
//			else if (length <= 0 && index <= 0) {
//				CloseCliSocket();
//			}
//			lstSend.pop_front();
//		}
//		
//	}
//	CloseCliSocket();
//}

bool CClinetSocket::Send(const CPacket& pack)
{
	if (m_clisock == -1)return false;
	std::string strOut = "";
	pack.Data(strOut);
	TRACE("Clinet send Data:%s\r\n", pack.strData);
	bool ret = send(m_clisock, strOut.c_str(), strOut.size(), 0);
	return ret;
}

void CClinetSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{//TODO:定义一个消息的数据结构(数据和数据长度，模式) 回调消息的的数据结构(HWND))
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	size_t nTemp = data.strData.size();
	CPacket current((BYTE*)data.strData.c_str(), nTemp);
	if (InitSocket() == true) {
		int ret = send(m_clisock, (char*)data.strData.c_str(), (int)data.strData.size(), 0);
		if (ret > 0) {
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_clisock != INVALID_SOCKET) {
				int length = recv(m_clisock, pBuffer + index, BUFFER_SIZE - index, 0);
				if (length > 0 || (index > 0)) {
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0) {
						TRACE("ack pack %d to hWnd %08X %d %d\r\n", pack.sCmd, hWnd, index, nLen);
						TRACE("%04X\r\n", *(WORD*)(pBuffer + nLen));
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);
						if (data.nMode & CSM_AUTOCLOSE) {
							CloseCliSocket();
							return;
						}
						index -= nLen;
						memmove(pBuffer, pBuffer + nLen, index);
					}
				}
				else {//TODO：对方关闭了套接字，或者网络设备异常
					TRACE("recv failed length %d index %d cmd %d\r\n", length, index, current.sCmd);
					CloseCliSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(current.sCmd, NULL, 0), 1);
				}
			}
		}
		else {
			CloseCliSocket();
			//网络终止处理
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else {
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
	}
}
