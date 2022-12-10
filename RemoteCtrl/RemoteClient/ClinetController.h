#pragma once
#include "ClinetSocket.h"
#include "WatchDlg.h"
#include "RemoteClientDlg.h"
#include "DlgStatus.h"
#include <map>
#include "resource.h"
#include "Tool.h"

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
	void UpdateAddr(int nIP, int nPort) {
		CClinetSocket::getInstance()->UpdateAddr(nIP, nPort);
	}
	int InitSocket() {
		CClinetSocket* pclient = CClinetSocket::getInstance();
		return  pclient->InitSocket();
	}
	int SendPack(const CPacket& pack) {
		CClinetSocket* pclient = CClinetSocket::getInstance();
		return pclient->Send(pack);
	}
	int DealCommand() {
		CClinetSocket* pclient = CClinetSocket::getInstance();		
		return pclient->DealCommand();
	}
	void CloseCliSocket() {
		CClinetSocket::getInstance()->CloseCliSocket();
	}
	int SendCmdPack(int nCmd, bool AutoClose = true,
		BYTE* pData = NULL, size_t nLength = 0) {			
		bool ret = InitSocket();
		if (ret == false) {
			AfxMessageBox("初始化失败！");
			return -1;
		}
		ret = SendPack(CPacket(nCmd, pData, nLength));
		if (ret == false) {
			TRACE("Client Test Send cmd is failed\r\n");
			return -2;
		}
		int cmd = DealCommand();
		//TRACE("recv the cmd is:%d\r\n", pclient->GetPacket().sCmd);
		if (AutoClose)
			CloseCliSocket();
		return cmd;
	}
	int Bytes2Image(CImage& image) {
		CClinetSocket* pclient = CClinetSocket::getInstance();
		return CTool::Bytes2Image(image, pclient->GetPacket().strData);
	}
	int DownFile(CString strPath) {
		CFileDialog dlg(false, NULL, strPath,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, &m_RemoteDlg);
		if (dlg.DoModal() == IDOK) {
			m_strRemote = strPath;
			m_strLocal = dlg.GetPathName();
			m_hThreadDown=(HANDLE*)_beginthread(&CClientController::threadEntryForDownFile, 0, this);
			if (WaitForSingleObject(m_hThreadDown, 0) == WAIT_TIMEOUT) {
				return -1;
			}
			m_RemoteDlg.BeginWaitCursor();
			m_StatusDlg.ShowWindow(SW_SHOW);
			m_StatusDlg.m_EditStatus.SetWindowText(_T("命令执行中......"));
			m_StatusDlg.SetActiveWindow();
			m_StatusDlg.CenterWindow();
		}
		return 0;
	}
	void StratWatchcreen() {
		m_isClosed = false;
		//CWatchDlg dlg(&m_RemoteDlg);
		m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadEntryForWatch, 0, this);
		m_RemoteDlg.DoModal();
		m_isClosed = true;
		WaitForSingleObject(m_hThreadWatch, 500);
	}
protected:
	static void threadEntryForWatch(void* arg);
	void threadWatch();
	static void threadEntryForDownFile(void* arg);
	void threadDownFile();
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
		m_isClosed = true;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_hThreadDown = INVALID_HANDLE_VALUE;
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}
	static void releaseInstance() {
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
			//CClientController::getInstance();
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
	HANDLE m_hThreadDown;
	CString m_strLocal;//文件本地路径
	CString m_strRemote;  //文件的远程路径
	HANDLE m_hThreadWatch;
	bool m_isClosed;//监视是否关闭
	unsigned int m_nThreadID;
	static CClientController* m_instance;
	static Helper m_helper;
};

