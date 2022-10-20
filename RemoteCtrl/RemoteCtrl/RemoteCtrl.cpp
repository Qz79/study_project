// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象
// 001测试分支切换演示
CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            // socket、bind、liten、accpet、read、write、close
            // 初始化环境，Windows下面有一个环境的初始化的，用到WSADATA
            WSADATA wsadata;
            WSAStartup(MAKEWORD(1, 1), &wsadata);//TODO:返回值需要处理
            SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);
            //TODO:返回值需要验证
            sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));//对结构体变量初始化
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = INADDR_ANY;
            serv_addr.sin_port = htons(9527);
            bind(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
            //TODO:返回值需要验证
            listen(serv_sock, 1);
            //TODO:返回值需要验证
            sockaddr_in cli_addr;
            memset(&cli_addr, 0, sizeof(cli_addr));
            int length = sizeof(cli_addr);
            SOCKET cli_sock=accept(serv_sock, (sockaddr*)&cli_addr, &length);
            char buffer[1024];
            memset(&buffer, '0', 1024);
            recv(cli_sock, buffer, sizeof(buffer), 0);
            send(cli_sock, buffer, sizeof(buffer), 0);
            closesocket(cli_sock);
            closesocket(serv_sock);
            WSACleanup();
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
