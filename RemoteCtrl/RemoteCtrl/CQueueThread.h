#pragma once
#include<list>
#include<string>

//������ɶ˿�IOCP��װ�̰߳�ȫ����

template <typename T>
class CQueueThread
{
public:
    typedef struct IocpParam {
        int nOperator;//����
        T strData;//����
        HANDLE hEvent;
        IocpParam(int op, const char* sData,HANDLE hEve==NULL) {
            nOperator = op;
            strData = sData;
        }
        IocpParam() {
            nOperator = -1;
        }
    }PPARAM;  //�������ݵĽṹ��
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

