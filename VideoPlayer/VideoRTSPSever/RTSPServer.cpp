#include "RTSPServer.h"

int RTSPServer::Init(const std::string& ip, short port)
{
    m_addr.Update(ip, port);
    m_sock.Bind(m_addr);
    m_sock.Listen();
    return 0;
}

int RTSPServer::Invoke()
{
    m_threadMain.Start();
    return 0;
}

void RTSPServer::Stop()
{
    m_sock.Close();
    m_threadMain.Stop();
    m_pool.Stop();
}

RTSPServer::~RTSPServer()
{
    Stop();
}

int RTSPServer::threadWorker()
{
    NetAddress addr;
    SOCKET clinet = m_sock.Accpect(addr);
    if (clinet != INVALID_SOCKET) {
        m_clients.PushBack(clinet);
        m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)RTSPServer::threadSession));
    } 
    return 0;
}

int RTSPServer::threadSession()
{ //TODO:接收数据请求 解析请求 应答请求
  //TODO:需要客户端的套接字，需要传参
    ZSocket client;//TODO:假设这里拿到了客户端对应的套接字
    EBuffer buffer(1024*16);
    int len=client.Recv(buffer);
    if (len <= 0) {
        //TODO:做清理client动作
        return -1;
    }
    buffer.resize(len);
    RTSPRequest req = AnalyseRequest(buffer);
    RTSPReply rep = MakeReply(req);
    client.Send(rep.toBuffer());
    return 0;
}
