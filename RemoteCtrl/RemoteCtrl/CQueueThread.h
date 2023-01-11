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
        int nOperator;//操作
        T strData;//数据
        HANDLE hEvent;
        IocpParam(int op, const char* sData) {
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
};

