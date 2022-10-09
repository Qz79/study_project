
#include <iostream>
#include"vlc.h"
#include<Windows.h>
std::string Unicode2Utf8(const std::wstring& strIn) {
	std::string str;
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), NULL, 0, NULL, NULL);
	str.resize(length + 1);
	::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), (LPSTR)str.c_str(), str.size(), NULL, NULL);
	return str;
}
int main()
{
	int argc = 1;
	char* argv[2];
	argv[0] = (char*)"--ignore-config";//多字节转Unicode,在转UTF-8
	libvlc_instance_t* vlc_ins = libvlc_new(argc, argv);
	std::string path = Unicode2Utf8(L"远程控制.mp4");
	libvlc_media_t* media = libvlc_media_new_path(vlc_ins, path.c_str());
	//libvlc_media_t* media = libvlc_media_new_location(vlc_ins, "file:///F:\Users\NINGMEI\\source\\study_project\\VideoPlayer\\VideoPlayer\\远程控制.mp4");
	libvlc_media_player_t* player = libvlc_media_player_new_from_media(media);
	do {
		int ret = libvlc_media_player_play(player);
		if (ret == -1) {
			printf("eeror found\r\n");
			break;
		}
		int vol = -1;
		while (vol == -1) {
			Sleep(10);
			vol = libvlc_audio_get_volume(player);
		}
		printf("volume is:%d\r\n", vol);
		libvlc_time_t tm = libvlc_media_player_get_length(player);//tm记录的是毫秒
		printf("%02d:%02d:%02d，%03d", int(tm / 3600000), int(tm / 60000) % 60, int(tm / 1000) % 60, int(tm % 1000));
		int width = libvlc_video_get_width(player);
		int height = libvlc_video_get_width(player);
		printf("width=%d,height=%d", width, height);
		getchar();
		libvlc_media_player_pause(player);
		getchar();
		libvlc_media_player_play(player);
		getchar();
		libvlc_media_player_stop(player);
	} while (0);
	libvlc_media_player_release(player);
	libvlc_media_release(media);
	libvlc_release(vlc_ins);
	return 0;
}
