// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "Command.h"
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


//int ExcuteCommand(int nCmd) {
//    int ret = 0;
//    switch (nCmd) {
//    case 1:
//        ret = MakeDirverInfo();
//        break;
//    case 2:
//        ret = MakeDirectoryInfo();
//        break;
//    case 3:
//        ret = RunFile();
//        break;
//    case 4:
//        ret = DownFile();
//        break;
//    case 5:
//        ret = MoueEvent();
//        break;
//    case 6:
//        ret = SendScreen();
//        break;
//    case 7:
//        ret = LockMachine();
//        /*Sleep(50);
//        LockMachine();*///T0:测试多次调用线程会不会产生问题
//        break;
//    case 8:
//        ret = UnLockMachine();
//        break;
//    case 9:
//        ret = DeleteLocalFile();
//    case 100:
//        ret = TestLink();
//        break;
//    }
//    /*T1:测试解锁
//    UnLockMachine();
//    TRACE("m_hWnd=%08x\r\n", dlg.m_hWnd);
//    while (dlg.m_hWnd != NULL) {
//        Sleep(10);
//    }*/
//    /*while ((dlg.m_hWnd != NULL)&&(dlg.m_hWnd != INVALID_HANDLE_VALUE))
//        Sleep(100);
//      T0: 测试多次调用线程会不会产生问题
//    */
//    return ret;
//}
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
            CCommand cmd;
            int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand,&cmd);
            switch (ret) {
            case -1:
                MessageBox(NULL, _T("网络初始化失败，请检查网络"), _T("提示"), MB_OK | MB_ICONERROR);
                break;
            case -2:
                MessageBox(NULL, _T("多次重试无效，结束程序"), _T("提示"), MB_OK | MB_ICONERROR);
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
