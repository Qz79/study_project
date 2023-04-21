#pragma once
#include"Socket.h"
#include<string>
#include"EdoyunThread.h"
#include<map>
#include"CEdoyunQueue.h"
class RTSPRequest {
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& request);
	RTSPRequest& operator= (const RTSPRequest& request);
	~RTSPRequest();
private:
	int m_method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4 TEARDOWN
};
class RTSPReply {
public:
	RTSPReply();
	RTSPReply(const RTSPReply& reply);
	EBuffer toBuffer();
	RTSPReply& operator= (const RTSPReply& reply);
	~RTSPReply();
private:
	int m_method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4 TEARDOWN
};

class RTSPSession{
public:
	RTSPSession(){}
	RTSPSession(const ZSocket& client);
	RTSPSession(const RTSPSession& session);
	int PickRequestAndReply();
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession();
private:
	ZSocket m_client;
	std::string m_id;
};
class RTSPServer:public ThreadFuncBase
{
public:
	RTSPServer():m_sock(true), status(0),m_pool(5){
		m_threadMain.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorker));
	}
	int Init(const std::string& ip = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
	~RTSPServer();
protected:
	//返回 0 继续，返回负数终止，返回其他警告
	int threadWorker();
	//RTSPRequest AnalyseRequest(const std::string& data);
	//RTSPReply MakeReply(const RTSPRequest& request);
	int threadSession();
private:
	ZSocket m_sock;
	int status;//0 未初始化 1 初始化完成 2 正在运营 3 关闭
	NetAddress m_addr;
	EdoyunThread m_threadMain;
	EdoyunThreadPool m_pool;
	//std::map<std::string, RTSPSession> m_mapSession;
	static SockerIniter init;
	CEdoyunQueue<RTSPSession> m_lstSession;
};

