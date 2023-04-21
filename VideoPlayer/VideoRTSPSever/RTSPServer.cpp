#include "RTSPServer.h"
#include"rpc.h"
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
        RTSPSession session(clinet);
        m_lstSession.PushBack(session);
        m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)RTSPServer::threadSession));
    } 
    return 0;
}
/*在接收请求的时候直接构造RTSPSession和clinet套接字绑定，同时为每个Session搞上独有的ID，最后将其插入到安全队列中*/
int RTSPServer::threadSession()
{ 
    RTSPSession session;
    if (m_lstSession.PopFront(session)) {
        return session.PickRequestAndReply();
    }
    return -1;
}

RTSPSession::RTSPSession(const ZSocket& client)
    :m_client(client)
{
    UUID uid;
    UuidCreate(&uid);
    m_id.resize(8);
    snprintf((char*)m_id.c_str(),m_id.size(), "08%d", uid.Data1);
}
