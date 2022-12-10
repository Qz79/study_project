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

void CClinetSocket::threadEntryFunc(void* arg)
{
	CClinetSocket* thiz = (CClinetSocket*)arg;
	thiz->threadFunc();
	_endthread();
}

void CClinetSocket::threadFunc()
{
	if (InitSocket() == false)return;
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	while (m_clisock != INVALID_SOCKET) {
		if (lstSend.size() > 0) {
			CPacket& head = lstSend.front();
			if (Send(head) == false) {
				TRACE("发送失败\r\n");
				continue;
			}
			auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list < CPacket>()));
			int length = recv(m_clisock, pBuffer, BUFFER_SIZE - index, 0);
			if (length > 0 || index > 0) {
				index += length;
				size_t size = (size_t)index;
				CPacket pack((BYTE*)pBuffer, size);	
				if (size > 0) {
					//TODO:通知对应的事件
					pack.hEvent = head.hEvent;
					pr.first->second.push_back(pack);
					SetEvent(head.hEvent);
					
				}
				continue;
			}
			else if (length <= 0 && index <= 0) {
				CloseCliSocket();
			}
			lstSend.pop_front();
		}
		
	}
}
