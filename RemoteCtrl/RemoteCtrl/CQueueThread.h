#pragma once
                                        /*《对这个此类封装的理解注释版本》*/
#include<list>
#include<string>
#include<atomic>
#include "pch.h"
#include<thread>
                                        /*《对这个此类封装的理解注释版本》*/
//利用完成端口IOCP封装线程安全队列
template <typename T>
class CQueueThread
{ //线程安全队列，利用（hIocp）
public:
    enum {
        IocpListNone,
        IocpListPush,
        IocpListPop,
        IocpListSize,
        IocpListClear
    };
    //向完成端口投递信息的结构体
    typedef struct IocpParam {
        size_t nOperator;//操作
        T Data;//数据
        HANDLE hEvent;
        IocpParam(size_t op, const T& sData,HANDLE hEve=NULL) {
            nOperator = op;
            Data = sData;
            hEvent = hEve;
        }
        IocpParam() {
            nOperator = IocpListNone;
        }
    }PPARAM;  
public:
    CQueueThread() {
        m_lock = false;
        m_hCompeletionPort= CreateIoCompletionPort(
            INVALID_HANDLE_VALUE, NULL, NULL, 1);
        m_nThread = INVALID_HANDLE_VALUE;
        if (m_hCompeletionPort != NULL) {
            m_nThread = (HANDLE)_beginthread(
                &CQueueThread<T>::threadQueueEntry, 
                0, this);
        }
    }
    ~CQueueThread() {
        if (m_lock)return;
        m_lock = true;
        PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);
        WaitForSingleObject(m_nThread, INFINITE);
        if (m_hCompeletionPort != NULL) {
            HANDLE Temp = m_hCompeletionPort;         
            m_hCompeletionPort = NULL;
            CloseHandle(Temp);
        }  
    }
    bool PushBack(const T& data) {
        IocpParam* pParam = new IocpParam(IocpListPush, data);
        if (m_lock) {
            delete pParam;
            return false;
        }
        // 将构造好的信息投递进完成端口，投递动作完成后此时线程启动
        // 线程将会处理投递进来的信息，将其插入到m_lstData
        bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, 
            sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
        if (ret == false)delete pParam;
        return ret;
    }
    bool PopFront(T& data) {   
        HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        IocpParam pParam (IocpListPop, data, hEvent);
        if (m_lock == true) {
            if (hEvent)CloseHandle(hEvent);
            return false;
        }
        bool ret = PostQueuedCompletionStatus(m_hCompeletionPort,
            sizeof(PPARAM), (ULONG_PTR)&pParam, NULL);
        if (ret == false) {
            CloseHandle(hEvent);
            return false;
        }
        ret = WaitForSingleObject(hEvent,INFINITE) == WAIT_OBJECT_0;
        if (ret) 
            data = pParam.Data;
        return ret;
    }
    size_t Size() {
        HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        IocpParam Param(IocpListSize, T(), hEvent);
        if (m_lock == true) {
            if (hEvent)CloseHandle(hEvent);
            return -1;
        }
        bool ret = PostQueuedCompletionStatus(m_hCompeletionPort,
            sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
        if (ret == false) {
            CloseHandle(hEvent);
            return -1;
        }
        ret = WaitForSingleObject(hEvent,INFINITE) == WAIT_OBJECT_0;
        if (ret) 
            return Param.nOperator;
        return -1;
    }
   bool Clear() {     
        if (m_lock)return false;
        IocpParam* pParam = new IocpParam(IocpListClear, T());
        bool ret = PostQueuedCompletionStatus(m_hCompeletionPort,
            sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
        if (ret == false)delete pParam;
        return ret;
    }
private:
    //此线程就是为了处理投递进完成端口的数据，以及从完成端口取数据
    static void threadQueueEntry(void* arg) {
        CQueueThread<T> *thiz = (CQueueThread<T>*) arg;
        thiz->threadWork();
        _endthread();
    }
    void DealParam(PPARAM* pParam) {
        switch (pParam->nOperator) {
        case IocpListPush:
            m_lstData.push_back(pParam->Data);
            delete pParam;
            break;
        case IocpListPop:
            if (m_lstData.size() > 0) {
                pParam->Data = m_lstData.front();
                m_lstData.pop_front();
            }
            if (NULL != pParam->hEvent) SetEvent(pParam->hEvent);
            break;
        case IocpListSize:
            pParam->nOperator = m_lstData.size();
            if (NULL != pParam->hEvent)   SetEvent(pParam->hEvent);
            break;
        case IocpListClear:
            m_lstData.clear();
            delete pParam;
            break;
        default:
            OutputDebugStringA("unkonw operator\r\n");
            break;
        }
    }
    void threadWork() {
        IocpParam* pParam = NULL;
        DWORD dwTransferred = 0;
        ULONG_PTR CompletionKey = 0;
        OVERLAPPED* pOverLapped = NULL;
        while (GetQueuedCompletionStatus(m_hCompeletionPort,
            &dwTransferred, &CompletionKey,
            &pOverLapped, INFINITE)) {
            if ((dwTransferred == 0) || (CompletionKey == NULL))
            {
                printf("thread is prepare to exit\r\n");
                break;
            }
            pParam = (PPARAM*)CompletionKey;
            DealParam(pParam);
        }
        //第二次做while的作用是？
        while (GetQueuedCompletionStatus(m_hCompeletionPort,
                &dwTransferred, &CompletionKey,
            &pOverLapped, 0)) {
            if ((dwTransferred == 0) || (CompletionKey == NULL)) {
                printf("thread is prepare to exit\r\n");
                continue;
            }
            pParam = (PPARAM*)CompletionKey;
            DealParam(pParam);
        }
        HANDLE Temp = m_hCompeletionPort;
        m_hCompeletionPort = NULL;
        CloseHandle(Temp);
    }
private:
	std::list<T> m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_nThread;
    std::atomic<bool> m_lock;
};
/*问题:
1.直接封装一个线程进行对std::list的数据直接插入和删除，有什么区别？
在多线程中使用list会挂，因为list是要先修改size,再进行插入，
当线程多起来后，可能会出现size修改完还没来得及插入数据，另一个线程就又来访问了
所以可以尝试对list加锁处理，进行对比。
2.完成端口究竟起到了什么作用？
3.工作线程的第一while和第二个while的区别？
4.工作线程的第二个while的作用是什么？
*/