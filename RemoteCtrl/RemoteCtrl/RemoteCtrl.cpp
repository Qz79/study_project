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
enum {
    IocpListEmpty,
    IocpListPush,
    IocpListPop
};
using namespace std;
typedef struct IocpParam {
    int nOperator;//操作
    std::string strData;//数据
    _beginthread_proc_type cbFunc;//回调
    IocpParam(int op, const char* sData, _beginthread_proc_type cb=NULL) {
        nOperator = op;
        strData = sData;
        cbFunc = cb;
    }
    IocpParam() {
        nOperator = -1;
    }
}IOCP_PARAM;
void threadWork(HANDLE hIOCP) {
    list<std::string> lstString;
    DWORD dwTransferred = 0;
    ULONG_PTR CompletionKey = 0;
    OVERLAPPED* pOverlapped = NULL;
    while (GetQueuedCompletionStatus(hIOCP, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE)) {
        if ((dwTransferred == 0) || (CompletionKey == NULL)) {
            printf("thread is prepare to exit\r\n");
            break;
        }
        IOCP_PARAM* pParam = (IOCP_PARAM*)CompletionKey;
        if (pParam->nOperator == IocpListPush) {
            lstString.push_back(pParam->strData);
        }
        else if (pParam->nOperator == IocpListPop) {
            string str;
            if (lstString.size() > 0) {
                str = lstString.front();
                lstString.pop_front();
            }
            if (pParam->cbFunc) {
                pParam->cbFunc(&str);
            }
        }
        else if (pParam->nOperator == IocpListEmpty) {
            lstString.clear();
        }
        delete pParam;
    }
}
void threadQueueEntry(HANDLE hIOCP) {
    threadWork(hIOCP);
    _endthread();
}
void func(void* arg) {
    string* pstr = (string*)arg;
    if (pstr != NULL) {
        printf("pop from list:%s\r\n", pstr->c_str());
        //delete pstr;
    }
    else {
        printf("list is empty,no data!\r\n");
    }
    
}
int main()
{
    //利用完成端口写一个线程安全的队列
    HANDLE hIOCP = INVALID_HANDLE_VALUE;
    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
    if ((hIOCP == INVALID_HANDLE_VALUE) || (hIOCP == NULL)) {
        printf("create hiop is failed %d\n", GetLastError());
        return 1;
    }
    HANDLE hThread=(HANDLE)_beginthread(threadQueueEntry, 0, hIOCP);
    printf("press any key to exit...\n");
    ULONGLONG tick = GetTickCount64();
    ULONGLONG tick0= GetTickCount64();
    while (_kbhit() == 0) {
        if (GetTickCount64() - tick0 > 1300) {
            PostQueuedCompletionStatus(hIOCP, sizeof(IOCP_PARAM), (ULONG_PTR) new IOCP_PARAM(IocpListPop, "Hello world!", func), NULL);
            tick0 = GetTickCount64();
        }
        if (GetTickCount64() - tick > 2000) {
            PostQueuedCompletionStatus(hIOCP, sizeof(IOCP_PARAM),(ULONG_PTR) new IOCP_PARAM(IocpListPush,"Hello world!"), NULL);
            tick = GetTickCount64();
        }
        Sleep(1);
    }
    if (hIOCP != NULL) {
        //TODO:唤醒完成端口
        PostQueuedCompletionStatus(hIOCP, 0, NULL, NULL);
        WaitForSingleObject(hThread, INFINITE);
    }
    CloseHandle(hIOCP);
    printf("exit done\n");
    ::exit(0);
    //int nRetCode = 0;

    //HMODULE hModule = ::GetModuleHandle(nullptr);

    //if (hModule != nullptr)
    //{
    //    // 初始化 MFC 并在失败时显示错误
    //    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
    //    {
    //        // TODO: 在此处为应用程序的行为编写代码。
    //        wprintf(L"错误: MFC 初始化失败\n");
    //        nRetCode = 1;
    //    }
    //    else
    //    {
    //        CCommand cmd;
    //        int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand,&cmd);
    //        switch (ret) {
    //        case -1:
    //            MessageBox(NULL, _T("网络初始化失败，请检查网络"), _T("提示"), MB_OK | MB_ICONERROR);
    //            break;
    //        case -2:
    //            MessageBox(NULL, _T("多次重试无效，结束程序"), _T("提示"), MB_OK | MB_ICONERROR);
    //            break;
    //        }   
    //    }
    //}
    //else
    //{
    //    // TODO: 更改错误代码以符合需要
    //    wprintf(L"错误: GetModuleHandle 失败\n");
    //    nRetCode = 1;
    //}

    //return nRetCode;
    return 0;
}
