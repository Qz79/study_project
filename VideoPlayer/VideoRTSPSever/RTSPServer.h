#pragma once
#include"Socket.h"
#include<string>
#include"EdoyunThread.h"
#include<map>
class RTSPRequest {
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& protocol);
	RTSPRequest& operator= (const RTSPRequest& protocol);
	~RTSPRequest();
private:
	int m_method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4 TEARDOWN
};
class RTSPReply {
public:
	RTSPReply();
	RTSPReply(const RTSPReply& request);
	RTSPReply& operator= (const RTSPReply& request);
	~RTSPReply();
private:
	int m_method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4 TEARDOWN
};
class RTSPSession{
public:
	RTSPSession();
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession();
};
class RTSPServer:public ThreadFuncBase
{
public:
	RTSPServer():m_sock(true), status(0){}
	int Init(const std::string& ip = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
	~RTSPServer();
protected:
	int threadWorker();
	RTSPRequest AnalyseRequest(const std::string& data);
	RTSPReply MakeReply(const RTSPRequest& request);
private:
	ZSocket m_sock;
	int status;//0 未初始化 1 初始化完成 2 正在运营 3 关闭
	EdoyunThread m_threadMain;
	EdoyunThreadPool m_pool;
	std::map<std::string, RTSPSession> m_mapSession;
};

