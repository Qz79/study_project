#include "pch.h"
#include "ClinetController.h"
CClientController* CClientController::m_instance = NULL;

int CClientController::Invoke(CWnd* pMainWnd)
{
	pMainWnd = &m_RemoteDlg;
	return m_RemoteDlg.DoModal();
}

int CClientController::InitController()
{
	hThread =(HANDLE) _beginthreadex(NULL, 0,
		&CClientController::threadEntry,
		this, 0, &m_nThreadID);

	return 0;
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
}

void CClientController::threadFunc()
{
}
