#pragma once
#include<list>
#include<string>
#include<atomic>
#include "pch.h"

//利用完成端口IOCP封装线程安全队列

template <typename T>
class CQueueThread
{//线程安全队列，利用（hIocp）
public:
    enum {
        IocpListNone,
        IocpListPush,
        IocpListPop,
        IocpListSize,
        IocpListClear
    };
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
    }PPARAM;  //Post Param用于投递信息的结构体

public:
    CQueueThread() {
        m_lock = false;
        m_hCompeletionPort= CreateIoCompletionPort(
            INVALID_HANDLE_VALUE, NULL, NULL, 1);
        m_nThread = INVALID_HANDLE_VALUE;
        if (m_hCompeletionPort != NULL) {
            m_nThread = (HANDLE)_beginthread(
                &CQueueThread<T>::threadQueueEntry, 
                0, m_hCompeletionPort);
        }
    }
    ~CQueueThread() {
        if (m_lock)return;
        m_lock = true;
        HANDLE Temp = m_hCompeletionPort;
        PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);
        WaitForSingleObject(m_nThread, INFINITE);
        m_hCompeletionPort = NULL;
        CloseHandle(Temp);
    }
    bool PushBack(const T& data) {
        IocpParam* pParam = new IocpParam(IocpListPush, data);
        if (m_lock == true) {
            delete pParam;
            return false;
        }
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
        ret = WaitForSingleObject(hEvent, INFINTE) == WAIT_OBJECT_0;
        if (ret) data = pParam.Data;
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
            sizeof(PPARAM), (ULONG_PTR)&pParam, NULL);
        if (ret == false) {
            CloseHandle(hEvent);
            return -1;
        }
        ret = WaitForSingleObject(hEvent, INFINTE) == WAIT_OBJECT_0;
        if (ret) return Param.nOperator;
        return -1;
    }
   bool Clear() {
        IocpParam* pParam = new IocpParam(IocpListPush, T());
        if (m_lock == true)return false;
        bool ret = PostQueuedCompletionStatus(m_hCompeletionPort,
            sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
        if (ret == false)delete pParam;
        return ret;
    }
private:
    static void threadQueueEntry(void* arg) {
        CQueueThread<T>* thiz = CQueueThread<T>*arg;
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
                pParam->Data = m_lstData.front()
                    m_lstData.pop_front();
            }
            if (NULL != pParam->hEvent)SetEvent(pParam->hEvent;
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
            while (GetQueuedCompletionStatus(m_hCompeletionPort,
                &dwTransferred, &CompletionKey,
                &pOverLapped, INFINITE)) {
                if ((dwTransferred == 0) || (CompletionKey == NULL)){
                    printf("thread is prepare to exit\r\n");
                    continue;
                }
                pParam = (PPARAM*)CompletionKey;
                DealParam(pParam);
        }
        CloseHandle(m_hCompeletionPort);
    }

private:
	std::list<T> m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_nThread;
    std::atomic<bool> m_lock;
};

