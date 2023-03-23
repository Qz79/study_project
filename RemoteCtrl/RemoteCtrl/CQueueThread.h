#pragma once
#include<list>
#include<string>

//利用完成端口IOCP封装线程安全队列

template <typename T>
class CQueueThread
{
public:
    typedef struct IocpParam {
        int nOperator;//操作
        T strData;//数据
        HANDLE hEvent;
        IocpParam(int op, const char* sData,HANDLE hEve==NULL) {
            nOperator = op;
            strData = sData;
        }
        IocpParam() {
            nOperator = -1;
        }
    }PPARAM;  //传输数据的结构体
    enum {
        IocpListPush,
        IocpListPop,
        IocpListSize,
        IocpListClear
    };
public:
    CQueueThread() {
        m_hCompeletionPort= CreateIoCompletionPort(
            INVALID_HANDLE_VALUE, NULL, NULL, 1);
        m_nThread = INVALID_HANDLE_VALUE;
        if (m_hCompeletionPort != NULL) {
            m_nThread = (HANDLE)_beginthread(threadQueueEntry, 0, m_hCompeletionPort);
        }
    }
    ~CQueueThread() {
        PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);
        WaitForSingleObject(m_nThread, INFINITE);
    }
	bool PushBack();
	bool PopFront();
	size_t Size();
	void Clear();
private:
    static void threadQueueEntry(void* arg);
    void threadWork();
private:
	std::list<T> m_Queue;
	HANDLE m_hCompeletionPort;
	HANDLE m_nThread;
  
};

