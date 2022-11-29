#pragma once
#include"resource.h"
#include"RemoteClientDlg.h"
#include"WatchDlg.h"
#include"DlgStatus.h"
#include<map>
//#define WM_SEND_PACK (WM_USER+2)
//#define WM_SEND_DATA (WM_USER+3)
#define WM_SEND_STATUS (WM_USER+2)
#define WM_SEND_WATCH (WM_USER+3)
#define WM_SEND_MESSAGE (WM_USER+0x1000)
class CClientController
{
public:
	static CClientController* getInstance();
	int Invoke(CWnd* pMainWnd);
	int InitController();
protected:
	static unsigned __stdcall threadEntry(void* arg);
	//控制层的线程启动，这里参数需要考虑传递到视图层和模型层，那么利用消息机制的话就不用去传递参数？
	void threadFunc();
	LRESULT OnSendStatus(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendWatch(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OSendMessage(MSG msg);
	typedef struct info {
		MSG msg;
		LRESULT result;
		info(MSG m) {
			result = -1;
			memcpy(&msg, &m, sizeof(MSG));
		}
		info(const info& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		info& operator=(const info& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;
	CClientController():m_WatchDlg(&m_RemoteDlg), m_StatusDlg(&m_RemoteDlg)
	{
		m_hThread = INVALID_HANDLE_VALUE;
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
	typedef LRESULT(CClientController::* MSGFUNC)(UINT msg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapMsgFunc;
	CRemoteClientDlg m_RemoteDlg;
	CWatchDlg m_WatchDlg;
	CDlgStatus m_StatusDlg;
	HANDLE m_hThread;
	unsigned int m_nThreadID;
	static CClientController* m_instance;
	static Helper m_helper;
};

