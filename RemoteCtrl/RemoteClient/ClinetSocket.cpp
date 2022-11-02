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