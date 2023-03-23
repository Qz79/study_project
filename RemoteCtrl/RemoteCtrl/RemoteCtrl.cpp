// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "Command.h"
#include<conio.h>
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
void threadQueueEntry(HANDLE hIocp) {
    //HANDLE hIocp==void* arg
    //该线程对hIocp的里面的数据获取
    DWORD dwTransferred = 0;
    ULONG_PTR CompletionKey = 0;
    OVERLAPPED* pOverLapped = NULL;
    while (GetQueuedCompletionStatus(hIocp, &dwTransferred, &CompletionKey, &pOverLapped, INFINITE)) {
        if ((dwTransferred == 0) && (CompletionKey == NULL))
        {
            printf("thread is prepare to exit\r\n");
            break;
        }
    }
    _endthread();
}
int main()
{
    if (!CTool::init())return 1;
    //1.声明并初始化一个句柄
    HANDLE hIocp = INVALID_HANDLE_VALUE;
    //2.创建完成端口对象
    hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
    //3.创建线程对完成端口对象进行队列式的获取和添加
    printf("press any key to exit....\r\n");
    HANDLE nThread=(HANDLE)_beginthread(threadQueueEntry, 0, hIocp);
    //4.判断完成端口对象，并向其投递信息
    getchar();
    while (hIocp) {
        PostQueuedCompletionStatus(hIocp, 0, NULL, NULL);
        WaitForSingleObject(nThread, INFINITE);
    }
    CloseHandle(hIocp);
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
