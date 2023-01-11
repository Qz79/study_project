#pragma once
#include<list>
#include<string>
template <typename T>
class CQueueThread
{
public:
	CQueueThread();
	~CQueueThread();
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
    typedef struct IocpParam {
        int nOperator;//����
        T strData;//����
        HANDLE hEvent;
        IocpParam(int op, const char* sData) {
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
};

