#pragma once
#include"resource.h"
#include"RemoteClientDlg.h"
#include"WatchDlg.h"
#include"DlgStatus.h"
#include<map>
class CClientController
{
public:
	static CClientController* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CClientController();
		}
		return m_instance;
	}
	int Invoke(CWnd* pMainWnd);
	int InitController();
protected:
	static unsigned __stdcall threadEntry(void* arg);
	//控制层的线程启动，这里参数需要考虑传递到视图层和模型层，那么利用消息机制的话就不用去传递参数？
	void threadFunc();
	CClientController():m_WatchDlg(&m_RemoteDlg), m_StatusDlg(&m_RemoteDlg)
	{
		hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {

	}
	static CClientController* releaseInstance() {
		if (m_instance != NULL) {
			CClientController* temp = m_instance;
			m_instance = NULL;
			delete temp;
		}
	}
private:
	class Helper {
	public:
		Helper() {
			CClientController::getInstance();
		}
		~Helper() {
			CClientController::releaseInstance();
		}

	};
private:
	CRemoteClientDlg m_RemoteDlg;
	CWatchDlg m_WatchDlg;
	CDlgStatus m_StatusDlg;
	HANDLE hThread;
	unsigned int m_nThreadID;
	static CClientController* m_instance;
	static Helper m_helper;
};

