// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include"ServerSocket.h"
#include<direct.h>
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

using namespace std;
void Dump(BYTE* pData, size_t nSize) {
    std::string strOut;
    for (size_t i = 0; i < nSize; i++) {
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0))strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);//&0xFF的操作是为了防止符号位产生负数
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}
int MakeDirverInfo() {
    /*
    int _chdrive(int drive);
    作用：更改当前工作驱动
    参数：drive 指定当前工作驱动器的 1 到 26 的整数（1=A，2=B 等）
    返回值：如果已成功更改当前工作驱动器，则为零 (0)，否则为 -1
    */
    std::string result;
    for (int i = 1; i <= 26; i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0) {
                result += ',';
            }
            result += 'A' + i - 1;
        }
    }
    result += ',';
    CPacket packet(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)packet.Data(), packet.Size());
    //CServerSocket::getInstance()->Send(packet);
    return 0;
}
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
        //    // TODO: 在此处为应用程序的行为编写代码。
        //    // socket、bind、liten、accpet、read、write、close
        //    // 初始化环境，Windows下面有一个环境的初始化的，用到WSADATA
        //    CServerSocket*pserver=CServerSocket::getInstance();
        //    if (pserver->InitSocket() == false) {
        //        MessageBox(NULL, _T("网络初始化失败，请检查网络"), _T("提示"), MB_OK | MB_ICONERROR);
        //        exit(0);
        //    }
        //    int count = 0;
        //    while (CServerSocket::getInstance() != NULL) {
        //        if (pserver->AcceptClient() == false) {
        //            if (count >= 3) {
        //                MessageBox(NULL, _T("多次重试无效，结束程序"), _T("提示"), MB_OK | MB_ICONERROR);
        //                exit(0);
        //            }
        //            MessageBox(NULL, _T("客户端连接异常,自动重试中"), _T("提示"), MB_OK | MB_ICONERROR);
        //            count++;
        //        }  
        //        int ret = pserver->DealCommand();//TODO:
        //    }
            int nCmd = 1;
            switch (nCmd) {
            case 1:
                MakeDirverInfo();
                break;
            }
            
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
