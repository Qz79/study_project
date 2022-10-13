#include "pch.h"
#include "CVlc.h"

CVlc::CVlc()
{
    m_instance= libvlc_new(0,NULL);
    m_media = NULL;
    m_player = NULL;
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
        libvlc_media_release(m_media);
    }
    if (m_instance != NULL) {
        libvlc_instance_t* temp = m_instance;
        m_instance = NULL;
        libvlc_release(m_instance);
    }
}

int CVlc::SetMedia(const std::string& strUrl)
{
    if (m_instance == NULL)return -1;
    m_media= libvlc_media_new_location(m_instance, strUrl.c_str());
    if (m_media == NULL)return -2;
    m_player = libvlc_media_player_new_from_media(m_media);
    if (m_player == NULL)return -3;
    return 0;
}

int CVlc::SetHwnd(HWND hWnd)
{
    if (!m_instance || !m_media || !m_player)return -1;
    libvlc_media_player_set_hwnd(m_player, hWnd);
    return 0;
}

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
    if (!m_instance || !m_media || !m_player)return -1.0;
    return libvlc_media_player_get_position(m_player);
}

VlcSize CVlc::GetMediaInfo()
{
    if (!m_instance || !m_media || !m_player)return VlcSize(-1,-1);
    return VlcSize(
        libvlc_video_get_width(m_player) , 
        libvlc_video_get_height(m_player)
    );
}
