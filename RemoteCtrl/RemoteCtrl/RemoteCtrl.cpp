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
void udp_server();
void udp_client(bool isHost = true);
int main(int argc,char* argv[])
{
    if (!CTool::Init())return 1;
    if (argc == 1) {
        char wstrDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, wstrDir);
        STARTUPINFOA si;
        memset(&si, 0, sizeof(si));
        PROCESS_INFORMATION pi;
        memset(&pi, 0, sizeof(pi));
        std::string strCmd = argv[0];
        strCmd += " 1";
        BOOL bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
        if (bRet) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            TRACE("进程ID：%d\r\n", pi.hProcess);
            TRACE("线程ID：%d\r\n", pi.hThread);
            strCmd += " 2";
            bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
            if (bRet) {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                TRACE("进程ID：%d\r\n", pi.hProcess);
                TRACE("线程ID：%d\r\n", pi.hThread);
                udp_server();//服务器
            }
        }
    }
    else if (argc == 2) {//主客户端
       udp_client();
    }
    else {//从客户端
        udp_client(false);
    }
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
void udp_server() {
    printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    getchar();
}
void udp_client(bool isHost) {
    if (isHost) {
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    }
    else {
        printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
    }
}