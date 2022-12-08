#include "pch.h"
#include "ClinetController.h"
CClientController* CClientController::m_instance = NULL;
std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapMsgFunc;
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

void CClientController::threadEntryForWatch(void* arg)
{

	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatch();
	_endthread();
}

void CClientController::threadWatch()
{
	Sleep(50);
	ULONGLONG nTick = GetTickCount64();
	while (!m_isClosed) {
		if (m_RemoteDlg.isFull() == false) {
			if (GetTickCount64() - nTick < 200) {
				Sleep(200 - DWORD(GetTickCount64() - nTick));
			}
			nTick = GetTickCount64();
			int ret = SendCmdPack(6);
			if (ret == 1) {
				//TRACE("成功发送请求图片命令\r\n");
			}
			else {
				TRACE("获取图片失败！ret = %d\r\n", ret);
			}
		}
		Sleep(1);
	}
	TRACE("thread end %d\r\n", m_isClosed);
}

void CClientController::threadEntryForDownFile(void* arg)
{
	CClientController* thiz = (CClientController * )arg;
	thiz->threadDownFile();
	_endthread();
}

void CClientController::threadDownFile()
{
	FILE* pfile = fopen(m_strLocal, "wb+");
	if (pfile == NULL) {
		AfxMessageBox(_T("本地没有权限保存文件或无法创建"));
		m_StatusDlg.ShowWindow(SW_HIDE);
		m_RemoteDlg.EndWaitCursor();
		return;
	}
	CClinetSocket* pclient = CClinetSocket::getInstance();
	do {
		int ret = SendCmdPack(4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength());
		if (ret < 0) {
			AfxMessageBox(_T("发送命令失败！"));
			TRACE("send four cmd failed:%d\r\n", ret);
			break;
		}

		long long nLength = *(long long*)pclient->GetPacket().strData.c_str();
		if (nLength == 0) {
			AfxMessageBox(_T("文件长度为零或无法打开"));
			break;
		}
		long long count = 0;
		while (count < nLength) {
			ret = pclient->DealCommand();
			if (ret < 0) {
				AfxMessageBox(_T("传输失败！"));
				TRACE("DealCmd failed:%d\r\n", ret);
				break;
			}
			fwrite(pclient->GetPacket().strData.c_str(), 1,
				pclient->GetPacket().strData.size(), pfile);
			count += pclient->GetPacket().strData.size();
		}
	} while (false);
	fclose(pfile);
	pclient->CloseCliSocket();
	m_StatusDlg.ShowWindow(SW_HIDE);
	m_RemoteDlg.EndWaitCursor();
	m_RemoteDlg.MessageBox(_T("下载完成"), _T("完成"));
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
//对此函数还是存在疑惑
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

