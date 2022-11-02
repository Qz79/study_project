#include "pch.h"
#include "ServerSocket.h"

CServerSocket* CServerSocket::m_istance = NULL;
CServerSocket::Helper CServerSocket::m_helper;
CServerSocket* pserver = CServerSocket::getInstance();