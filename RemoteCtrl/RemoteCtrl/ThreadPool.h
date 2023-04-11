#pragma once
#include<thread>
#include <afxwin.h>
#include<atomic>
#include<vector>
#include<mutex>
/*此线程存在弊端：
就是当线程结束的时候，要重新再给此线程布置任务的时候，就又要重新开启一个线程
而开线程是消耗一定时间的，所以这个类是需要改进的
改进的方式：
有事情做的时候就做事情，没有事情做就让该线程空转，当真正需要结束的时候再去结束
*/
class ThreadFuncBase {};
typedef int(ThreadFuncBase::* FUNCTYPE)();
class ThreadWorker {
public:
	ThreadWorker() :thiz(NULL), func(NULL) {}
	ThreadWorker(ThreadFuncBase* obj, FUNCTYPE f):thiz((ThreadFuncBase*)obj),func(f) {}
	ThreadWorker(const ThreadWorker& work){
		//重载复制构造函数
		thiz = work.thiz;
		func = work.func;
	}
	ThreadWorker& operator=(const ThreadWorker& work){
		//等于号重载
		if (this != &work) {
			thiz = work.thiz;
			func = work.func;
		}
		return *this;
	}
	int operator()() {
		if (thiz) {
			return (thiz->*func)();
		}
		return -1;
	}
	bool isValid()const {
		return (thiz != NULL) && (func != NULL);
	}
private:
	ThreadFuncBase* thiz; //这步是为了什么，构造ThreadFuncBase类？
	FUNCTYPE func;
};
class CThread {
//创建基本线程的类，在此基础上給线程安排具体工作给到worker
public:
	CThread() {
		m_hThread = NULL;
		m_bStatus = false;
	}
	~CThread() {
		Stop();
	}
	bool Start() {
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(CThread::threadEntry, 0, this);
		if (!IsValid()) {
			m_bStatus = false;
		}
		return m_bStatus;
	}
	bool IsValid() {
		if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE)
			return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	}
	bool Stop() {
		if (m_bStatus == false)return true;
		m_bStatus = false;
		DWORD ret = WaitForSingleObject(m_hThread, 1000);
		if (ret == WAIT_TIMEOUT) {
			TerminateThread(m_hThread, -1);
		}
		UpdateWorker();
		return ret==WAIT_OBJECT_0;
	}
	void UpdateWorker(const ::ThreadWorker& worker=::ThreadWorker()) {
		if (m_worker.load() != NULL && (m_worker.load() != &worker)) {
			::ThreadWorker* pWorker = m_worker.load();
			TRACE("delete pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load());
			m_worker.store(NULL);
			delete pWorker;
		}
		if (m_worker.load() == &worker)return;
		if (!worker.isValid()) {
			m_worker.store(NULL);
			return;
		}
		::ThreadWorker* pWorker = new ::ThreadWorker(worker);
		TRACE("new pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load());
		m_worker.store(pWorker);
	}
	//true 表示空闲，false 表示已经分配了工作
	bool IsIdle() {
		if (m_worker.load() == NULL)return true;
		return !m_worker.load()->isValid();
	}
private:
	void threadWorker() {
		while(m_bStatus) {
			if (m_worker.load() == NULL) {
				Sleep(1);
				continue;
			}
			::ThreadWorker worker = *m_worker.load();
			if (worker.isValid()) {
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
					int ret = worker();
					if (ret != 0) {
						CString str;
						str.Format(_T("thread found warning code %d\r\n"), ret);
						OutputDebugString(str);
					}
					if (ret < 0) {
						::ThreadWorker* pWorker = m_worker.load();
						m_worker.store(NULL);
						delete pWorker;
					}
				}
			}
			else {
				Sleep(1);
			}
		}
	}
	static void threadEntry(void* arg) {
		CThread* thiz = (CThread*)arg;
		if (thiz)
			thiz->threadWorker();
		_endthread();
	}
private:
	HANDLE m_hThread;
	bool m_bStatus;//true 表示正在运行，false表示终止运行
	std::atomic<::ThreadWorker*> m_worker;
}; 
class ThreadPool {
public:
	ThreadPool(){}
	ThreadPool(size_t size) {
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++)
			m_threads[i] = new CThread();
	}
	~ThreadPool(){
		Stop();
		for (size_t i = 0; i < m_threads.size(); i++) {
			delete m_threads[i];
			m_threads[i] = NULL;
		}
		m_threads.clear();
	}
	bool InVoke() {
		int ret = true;
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->Start() == false) {
				ret = false;
				break;
			}
		}
		if (ret == false) {
			for (size_t i = 0; i < m_threads.size(); i++)
				m_threads[i]->Stop();
		}
		return ret;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++)
			m_threads[i]->Stop();
	}
	/*可优化：
	原因：当线程规模小可以用，当超过几百个，或是更多线程规模大就需要优化，
	否则遍历的时间会增加，消耗的时间也就增加
	这里分发任务可以做优化，就是把空闲的线程全部放到一个链表中*/
	//返回-1表示分配失败，所有线程都在忙，大于等于0，表示分配第几个线程
	int DispatchWorker(const ThreadWorker& worker) {
		//分发任务
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->IsIdle()) {
				m_threads[i]->UpdateWorker(worker);
				index = i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}
	bool checkThreadValid(size_t index) {
		if (index < m_threads.size()) {
			return m_threads[index]->IsValid();
		}
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<CThread*> m_threads;
};