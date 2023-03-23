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
//CWinApp theApp;
//要传递的数据结
enum {
    IocpListPush,
    IocpListPop,
    IocpListEntry
};
//模拟的数据
typedef struct IocpParam {
    int operate;                    //对该数据进行的操作
    std::string strData;            //数据
    _beginthread_proc_type cbFunc;  //处理数据的回调函数
    IocpParam(int op, std::string str, _beginthread_proc_type cb = NULL) {
        operate = op;
        strData = str;
        cbFunc = cb;
    }
    IocpParam() {
        operate = -1;
    }
}IOCP_PARAM;
//操作完成端口对象的线程
void threadWork(HANDLE hIocp) {
    // HANDLE hIocp == void* arg
     //该线程对hIocp的里面的数据获取
    std::list<std::string> strList;
    DWORD dwTransferred = 0;
    ULONG_PTR CompletionKey = 0;
    OVERLAPPED* pOverLapped = NULL;
    int count = 0, count0 = 0;
    while (GetQueuedCompletionStatus(hIocp, &dwTransferred, &CompletionKey, &pOverLapped, INFINITE)) {
        if ((dwTransferred == 0) || (CompletionKey == NULL))
        {
            printf("thread is prepare to exit\r\n");
            break;
        }
        IOCP_PARAM* Param = (IOCP_PARAM*)CompletionKey;
        if (IocpListPush == Param->operate) {
            strList.push_back(Param->strData);
            count++;
        }
        else if (IocpListPop == Param->operate) {
            std::string* str = NULL;
            if (strList.size() > 0) {
                str = new std::string(strList.front());
                strList.pop_front();
            }
            if (NULL != Param->cbFunc) {
                Param->cbFunc(str);
            }
            count0++;
        }
        else if (IocpListEntry == Param->operate) {
            strList.clear();
        }
        delete Param;
    }
    printf("thread is exit count:%d count0:%d\r\n", count, count0);
}
void threadQueueEntry(HANDLE hIocp) {
    threadWork(hIocp);//为防止下述情况发生，利用编译器做出调整
    _endthread();//线程到此为止，会导致一些对象无法调用自身的析构，从而导致内存泄漏
}
//对模拟数据处理的回调函数
void func(void* arg) {
    std::string* str = (std::string*)arg;
    if (str) {
        printf("pop from list:%s\r\n", str->c_str());
        delete str;
    }
    else {
        printf("strlist is empty,no data\r\n");
    }
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
    ULONGLONG tick = GetTickCount64();
    ULONGLONG tick0 = GetTickCount64();
    int count = 0, count0 = 0;
    while (_kbhit() == 0) {
        if (GetTickCount64() - tick0 > 1300) {//每隔1.3秒触发一次
            PostQueuedCompletionStatus(hIocp, sizeof(IOCP_PARAM), (ULONG_PTR)new IOCP_PARAM(IocpListPop, "hello world!",func), NULL);
            tick0 = GetTickCount64();
            count0++;
        }
        if (GetTickCount64() - tick > 2000) {//每隔2秒触发一次
            PostQueuedCompletionStatus(hIocp, sizeof(IOCP_PARAM), (ULONG_PTR)new IOCP_PARAM(IocpListPush,"hello world!"), NULL);
            tick= GetTickCount64();
            count++;
        }
        Sleep(1);
    }
   if (hIocp) {
        PostQueuedCompletionStatus(hIocp, 0, NULL, NULL);
        WaitForSingleObject(nThread, INFINITE);
    }
   printf("%d,%d", count, count0);
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
