#pragma once
#include<Windows.h>
#include<thread>
#include <afxwin.h>
class ThreadPool {
public:
	ThreadPool() {
		m_hThread = NULL;
		m_bStatus = false;
	}
	bool Start() {
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(ThreadPool::threadEntry, 0, this);
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
		return WaitForSingleObject(m_hThread, INFINITE)==WAIT_OBJECT_0;
	}
protected:
	//线程的具体工作安排交给用户去开发，返回值小于0终止线程循环,不等于0则警告
	virtual int each_step() = 0;		
private:
	void threadWorker() {
		while (m_bStatus) {
			int ret=each_step();
			if (ret != 0) {
				CString str;
				str.Format(_T("thread found warning code\r\n", ret));
				OutputDebugString(str);
			}
			if (ret < 0) {
				break;
			}	
		}
	}
	static void threadEntry(void* arg) {
		ThreadPool* thiz = (ThreadPool*)arg;
		if (thiz)
			thiz->threadWorker();
		_endthread();
	}
private:
	HANDLE m_hThread;
	bool m_bStatus;//true 表示正在运行，false表示终止运行
};
