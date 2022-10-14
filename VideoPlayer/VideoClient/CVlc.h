#pragma once
#include"vlc.h"
#include<string>
//对VLC库进行类的封装，在封装的过程中要考虑可移植性
class VlcSize {
public:
	VlcSize(int width=0, int height=0) {
		m_width = width;
		m_height = height;
	}
	VlcSize(const VlcSize& vlcsize) {
		m_width = vlcsize.m_width;
		m_height = vlcsize.m_height;
	}
	VlcSize& operator=(const VlcSize& vlcsize) {
		if (this != &vlcsize) {
			m_width = vlcsize.m_width;
			m_height = vlcsize.m_height;
		}
		return *this;
	}
private:
	int m_width;
	int m_height;
};
class CVlc
{
public:
	CVlc();
	~CVlc();
	int SetMedia(const std::string& strUrl);
	int Play();
	int Pause();
	int Stop();
	int SetVolume(int vol);
	int GetVolume();
	void SetPosition(float pos);
	float GetPosition();
	VlcSize GetMediaInfo();
	std::string Unicode2Utf8(const std::wstring& strIn);
	int SetHwnd(HWND hWnd);
private:
	libvlc_instance_t* m_instance;
	libvlc_media_t* m_media;
	libvlc_media_player_t* m_player;
};

//运算符重载和拷贝构造需要增加理解和练习