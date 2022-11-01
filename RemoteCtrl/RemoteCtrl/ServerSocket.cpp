#include "pch.h"
#include "ServerSocket.h"

ServerSocket* ServerSocket::m_istance = NULL;
ServerSocket::Helper ServerSocket::m_helper;
ServerSocket* pserver = ServerSocket::getInstance();