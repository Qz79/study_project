#pragma once
#include"Socket.h"
#include<string>
class RTSPServer
{
public:
	RTSPServer():m_sock(true), status(0){}
	int Init(const std::string& ip = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
	~RTSPServer();
private:
	ZSocket m_sock;
	int status;//0 未初始化 1 初始化完成 2 正在运营 3 关闭
};

