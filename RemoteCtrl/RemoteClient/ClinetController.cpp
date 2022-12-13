#include "pch.h"
#include "ClinetController.h"
CClientController* CClientController::m_instance = NULL;
std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapMsgFunc;
CClientController::Helper CClientController::m_helper;
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

bool CClientController::SendCommandPacket(HWND hWnd, int nCmd, bool bAutoClose, BYTE* pData, size_t nLength, WPARAM wParam)
{
	TRACE("cmd:%d %s start %lld \r\n", nCmd, __FUNCTION__, GetTickCount64());
	CClinetSocket* pClient = CClinetSocket::getInstance();
	bool ret = pClient->SendPacket(hWnd, CPacket(nCmd, pData, nLength), bAutoClose, wParam);
	return ret;
}

int CClientController::DownFile(CString strPath)
{
	CFileDialog dlg(
		FALSE, NULL,
		strPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, &m_RemoteDlg);
	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		FILE* pFile = fopen(m_strLocal, "wb+");
		if (pFile == NULL) {
			AfxMessageBox(_T("����û��Ȩ�ޱ�����ļ��������ļ��޷�����������"));
			return -1;
		}
		SendCommandPacket(m_RemoteDlg, 4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), (WPARAM)pFile);
		m_RemoteDlg.BeginWaitCursor();
		m_StatusDlg.m_EditStatus.SetWindowText(_T("��������ִ���У�"));
		m_StatusDlg.ShowWindow(SW_SHOW);
		m_StatusDlg.CenterWindow(&m_RemoteDlg);
		m_StatusDlg.SetActiveWindow();
	}
	return 0;
}

void CClientController::DownloadEnd()
{
	m_StatusDlg.ShowWindow(SW_HIDE);
	m_RemoteDlg.EndWaitCursor();
	m_RemoteDlg.MessageBox(_T("������ɣ���"), _T("���"));
}


void CClientController::StratWatchcreen()
{
	m_isClosed = false;
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadEntryForWatch, 0, this);
	m_WatchDlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
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
		if (m_WatchDlg.isFull() == false) {
			if (GetTickCount64() - nTick < 200) {
				Sleep(200 - DWORD(GetTickCount64() - nTick));
			}
			nTick = GetTickCount64();
			int ret = SendCommandPacket(m_WatchDlg.GetSafeHwnd(), 6, true, NULL, 0);
			if (ret == 1) {
				//TRACE("�ɹ���������ͼƬ����\r\n");
			}
			else {
				TRACE("��ȡͼƬʧ�ܣ�ret = %d\r\n", ret);
			}
		}
		Sleep(1);
	}
	TRACE("thread end %d\r\n", m_isClosed);
}

//void CClientController::threadEntryForDownFile(void* arg)
//{
//	CClientController* thiz = (CClientController * )arg;
//	thiz->threadDownFile();
//	_endthread();
//}

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
//�Դ˺������Ǵ����ɻ�
LRESULT CClientController::OSendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)return -2;
	MSGINFO info(msg);
	//WM_SEND_MESSAGE �˴�����Ϣ��Ϊ��ʲô��û����Ϣӳ�䣬�˲�������ɻ󣬶����¼�û���ͷ�
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, -1);
	return info.result;
}

