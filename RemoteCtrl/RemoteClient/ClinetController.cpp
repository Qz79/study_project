#include "pch.h"
#include "ClinetController.h"
CClientController* CClientController::m_instance = NULL;

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientController();
		struct { 
			UINT msg;
			MSGFUNC func;
		}msgFuncs[] { 
			{WM_SEND_STATUS,&CClientController::OnSendStatus},
			{WM_SEND_WATCH,&CClientController::OnSendWatch},
			{(UINT)-1,NULL} 
		};
		for (int i = 0; msgFuncs[i].msg != -1; i++) {
			m_mapMsgFunc.insert(std::pair<UINT, MSGFUNC>(msgFuncs[i].msg, msgFuncs[i].func));
		}	
	}
	return m_instance;
}

	int CClientController::Invoke(CWnd* pMainWnd)
{
	pMainWnd = &m_RemoteDlg;
	return m_RemoteDlg.DoModal();
}

int CClientController::InitController()
{
	m_hThread =(HANDLE) _beginthreadex(NULL, 0,
		&CClientController::threadEntry,
		this, 0, &m_nThreadID);

	return 0;
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		if (msg.message == WM_SEND_MESSAGE) {
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent =(HANDLE) msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapMsgFunc.find(pmsg->msg.message);
			if(it != m_mapMsgFunc.end()){
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else {
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else {
			std::map<UINT, MSGFUNC>::iterator it = m_mapMsgFunc.find(msg.message);
			if (it != m_mapMsgFunc.end()) {
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
	}
}


LRESULT CClientController::OnSendStatus(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return m_StatusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnSendWatch(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return m_WatchDlg.DoModal();
}

LRESULT CClientController::OSendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)return -2;
	MSGINFO info(msg);
	//WM_SEND_MESSAGE 此处的消息是为了什么，没有消息映射，此步骤存在疑惑，而且事件没有释放
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, -1);
	return info.result;
}

