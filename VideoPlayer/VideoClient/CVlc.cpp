#include "pch.h"
#include "CVlc.h"

CVlc::CVlc()
{
    m_instance= libvlc_new(0,NULL);
    m_media = NULL;
    m_player = NULL;
    m_hwnd = NULL;
}

CVlc::~CVlc()
{
    if (m_player != NULL) {
        libvlc_media_player_t* temp = m_player;
        m_player = NULL;
        libvlc_media_player_release(temp);
    }
    if (m_media != NULL) {
        libvlc_media_t* temp = m_media;
        m_media = NULL;
        libvlc_media_release(temp);
    }
    if (m_instance != NULL) {
        libvlc_instance_t* temp = m_instance;
        m_instance = NULL;
        libvlc_release(temp);
    }
}

int CVlc::SetMedia(const std::string& strUrl)
{
    if (m_instance == NULL||(m_hwnd==NULL))return -1;
    if (m_strUrl == strUrl)return 0;
    m_strUrl = strUrl;
    if (m_media != NULL) {
        libvlc_media_release(m_media);
        m_media = NULL;
    }
    m_media= libvlc_media_new_location(m_instance, strUrl.c_str());
    if (m_media == NULL)return -2;
    if (m_player != NULL) {
        libvlc_media_player_release(m_player);
        m_player = NULL;
    }
    m_player = libvlc_media_player_new_from_media(m_media);
    if (m_player == NULL)return -3;
    CRect rect;
    GetWindowRect(m_hwnd, rect);
    std::string strRatio = "";
    strRatio.resize(32);
    sprintf((char*)strRatio.c_str(), "%d:%d", rect.Width(), rect.Height());
    libvlc_video_set_aspect_ratio(m_player, strRatio.c_str());
    libvlc_media_player_set_hwnd(m_player, m_hwnd);
    return 0;
}
#ifdef WIN32
int CVlc::SetHwnd(HWND hWnd)
{
    m_hwnd = hWnd;
    return 0;
}
#endif // WIN32



int CVlc::Play()
{
    if (!m_instance || !m_media || !m_player)return -2;
    return libvlc_media_player_play(m_player);
}

int CVlc::Pause()
{
    if (!m_instance || !m_media || !m_player)return -1;
    libvlc_media_player_pause(m_player);
    return 0;
}

int CVlc::Stop()
{
    if (!m_instance || !m_media || !m_player)return -1;
    libvlc_media_player_stop(m_player);
    return 0;
}

int CVlc::SetVolume(int vol)
{
    if (!m_instance || !m_media || !m_player)return -1;
    return libvlc_audio_set_volume(m_player,vol);
}

int CVlc::GetVolume()
{
    if (!m_instance || !m_media || !m_player)return -1;
    return  libvlc_audio_get_volume(m_player);
}

void CVlc::SetPosition(float pos)
{
    if (!m_instance || !m_media || !m_player)return;
    libvlc_media_player_set_position(m_player,pos);
}

float CVlc::GetPosition()
{
    if (!m_instance || !m_media || !m_player)return -1.0f;
    return libvlc_media_player_get_position(m_player);
}

float CVlc::GetLength()
{
    if (!m_instance || !m_media || !m_player)return -1.0f;
    libvlc_time_t tm = libvlc_media_player_get_length(m_player);
    float ret = tm / 1000.0f;
    return ret;
}

VlcSize CVlc::GetMediaInfo()
{
    if (!m_instance || !m_media || !m_player)return VlcSize(-1,-1);
    return VlcSize(
        libvlc_video_get_width(m_player) , 
        libvlc_video_get_height(m_player)
    );
}

std::string CVlc::Unicode2Utf8(const std::wstring& strIn)
{
    std::string str;
    int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), NULL, 0, NULL, NULL);
    str.resize(length + 1);
    ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), (LPSTR)str.c_str(), str.size(), NULL, NULL);
    return str;
}
