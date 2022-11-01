#include "pch.h"
#include "ClinetSocket.h"
CClinetSocket* CClinetSocket::m_istance = NULL;
CClinetSocket::Helper CClinetSocket::m_helper;
CClinetSocket* pclient = CClinetSocket::getInstance();