#pragma once
#include"VideoClientDlg.h"
#include"CVlc.h"
enum VLCFUNC{
	VLC_PLAY,
	VLC_PAUSE,
	VLC_STOP,
	VLC_GET_VOLUME,
	VLC_GET_POSITION,
	VLC_GET_LENGTH,
};
class ClientController
{
public:
	ClientController();
	~ClientController();
	int Init(CWnd*& pWnd);
	int Invoke();
	//如果出现中文字符，请转换成UTF-8字符
	int SetMedia(const std::string& strUrl);
	float VideoCtrl(VLCFUNC cmd);
	int SetHwnd(HWND hWnd);
	int SetVolume(int vol);
	void SetPosition(float pos);
	std::string Unicode2Utf8(const std::wstring& strIn);
	VlcSize GetMediaInfo();
private:
	CVlc m_vlc;
	CVideoClientDlg m_videodlg;
};

