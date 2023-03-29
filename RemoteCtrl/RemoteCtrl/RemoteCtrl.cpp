// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "Command.h"
#include<conio.h>
#include"CQueueThread.h"
#include <mswsock.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 入口函数改变的代码形式：
//#pragma comment(linker,"/subsystem:windows/entry:WinMainCTRStartup")
//#pragma comment(linker,"/subsystem:windows/entry:mainCTRStartup")
//#pragma comment(linker,"/subsystem:console/entry:WinMainCTRStartup")
//#pragma comment(linker,"/subsystem:console/entry:mainCTRStartup")

// 唯一的应用程序对象
// 001测试分支切换演示
CWinApp theApp;

//void test() {
//    CQueueThread<std::string> lstString;
//    ULONGLONG tick0 = GetTickCount64(), tick = GetTickCount64(),total= GetTickCount64();
//    while ((GetTickCount64()-total)<=1000) {
//        if (GetTickCount64() - tick0 > 13) {//每隔1.3秒触发一次
//            lstString.PushBack("hello,world");
//            tick0 = GetTickCount64();
//
//        }
//        if (GetTickCount64() - tick > 20) {//每隔2秒触发一次
//            std::string str;
//            lstString.PopFront(str);
//            tick = GetTickCount64();
//            printf("pop from queue:%s\r\n", str.c_str());
//        }
//        Sleep(1);
//    }
//    printf("exit done! size:%d\n", lstString.Size());
//    lstString.Clear();
//    printf("exit done! size:%d\n", lstString.Size());
//}
class COverlapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;
    char m_buffer[4096];
    COverlapped() {
        m_operator = 0;
        memset(&m_overlapped, 0, sizeof(OVERLAPPED));
        memset(&m_buffer, 0, sizeof(m_buffer));
    }
};
void iocp(){
    SOCKET socksev = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if(INVALID_SOCKET==socksev){
        CTool::ShowError();
        return;
    }
    HANDLE hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, socksev, 4);
    CreateIoCompletionPort((HANDLE)socksev, hIocp, NULL, 0);
    SOCKET sockcli = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_addr.S_un.S_addr =inet_addr( "0.0.0.0");
    addr.sin_port = htons(9527);
    bind(socksev, (sockaddr*)&addr, sizeof(addr));
    listen(socksev, 5); 
    COverlapped overlapped;
    overlapped.m_operator = 1; //代表操作是accept
    DWORD received = 0;
    BOOL ret=AcceptEx(socksev, sockcli, overlapped.m_buffer, 0, sizeof(sockaddr_in) + 16, 
        sizeof(sockaddr_in) + 16, &received, &overlapped.m_overlapped);
    if(ret == FALSE) {
        CTool::ShowError();
        return;
    }
    overlapped.m_operator = 2;
    WSASend();
    overlapped.m_operator = 3;
    WSARecv();
    //假设开启一个线程
    while (true) {
        DWORD dwTransferred = 0;
        ULONG_PTR CompletionKey = 0;
        OVERLAPPED* pOverLapped = NULL;
        if (GetQueuedCompletionStatus(hIocp, &dwTransferred, &CompletionKey,
            &pOverLapped, INFINITE)) {
            COverlapped* op = CONTAINING_RECORD(pOverLapped, COverlapped, m_overlapped);
            switch (op->m_operator) {
            case 1:
                //处理accept
                break;
            }
        }
    }
}
int main()
{
    if (!CTool::Init())return 1;

    /*CCommand cmd;
    int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
    switch (ret) {
    case -1:
        MessageBox(NULL, _T("网络初始化失败，请检查网络"), _T("提示"), MB_OK | MB_ICONERROR);
        break;
    case -2:
        MessageBox(NULL, _T("多次重试无效，结束程序"), _T("提示"), MB_OK | MB_ICONERROR);
        break;
    }*/
    return 0;
}
