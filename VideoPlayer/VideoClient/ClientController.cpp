#include "pch.h"
#include "ClientController.h"

ClientController::ClientController()
{
	m_videodlg.m_controller = this;
}

ClientController::~ClientController()
{
}

int ClientController::Init(CWnd*& pWnd)
{
	pWnd = &m_videodlg;
	return 0;
}

int ClientController::Invoke()
{
	INT_PTR nResponse = m_videodlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}
	return nResponse;
}

int ClientController::SetMedia(const std::string& strUrl)
{
	return m_vlc.SetMedia(strUrl);
}

float ClientController::VideoCtrl(VLCFUNC cmd)
{
	switch (cmd) {
	case VLC_PLAY:
		return float(m_vlc.Play());
	case VLC_PAUSE:
		return float(m_vlc.Pause());
	case VLC_STOP:
		return float(m_vlc.Stop());
	case VLC_GET_VOLUME:
		return float(m_vlc.GetVolume());
	case VLC_GET_POSITION:
		return m_vlc.GetPosition();
	case VLC_GET_LENGTH:
		return m_vlc.GetLength();
	
	default:
		break;
	}
	return -1.0f;
}

int ClientController::SetHwnd(HWND hWnd)
{
	return m_vlc.SetHwnd(hWnd);
}

int ClientController::SetVolume(int vol)
{
	return m_vlc.SetVolume(vol);
}

void ClientController::SetPosition(float pos)
{
	m_vlc.SetPosition(pos);
}

std::string ClientController::Unicode2Utf8(const std::wstring& strIn)
{
	return m_vlc.Unicode2Utf8(strIn);
}

VlcSize ClientController::GetMediaInfo()
{
	return m_vlc.GetMediaInfo();
}
