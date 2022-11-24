#pragma once
#include<map>
#include"ServerSocket.h"
#include<direct.h>
#include<atlimage.h>
#include"Tool.h"
#include<io.h>
#include<list>
#include"CLockInFoDialg.h"
#include"resource.h"
/*业务功能函数放在.h下可随时替换更改，此类的核心功能还是在.cpp中，类似MFC的消息映射思想*/
class CCommand
{
public:
	CCommand();
	~CCommand(){}
	int ExcuteCommand(int nCmd);
protected:
    CLockInFoDialg m_dlg;
    unsigned m_threadid;
private:
	typedef int (CCommand::* FUNCCMD)();
	std::map<int, FUNCCMD> m_mapFunc;
private:
    int MakeDirverInfo()
    {
        /*
        int _chdrive(int drive);
        作用：更改当前工作驱动
        参数：drive 指定当前工作驱动器的 1 到 26 的整数（1=A，2=B 等）
        返回值：如果已成功更改当前工作驱动器，则为零 (0)，否则为 -1
        */
        std::string result;
        for (int i = 1; i <= 26; i++) {
            if (_chdrive(i) == 0) {
                if (result.size() > 0) {
                    result += ',';
                }
                result += 'A' + i - 1;
            }
        }
        result += ',';
        CPacket packet(1, (BYTE*)result.c_str(), result.size());
        CTool::Dump((BYTE*)packet.Data(), packet.Size());
        CServerSocket::getInstance()->Send(packet);
        return 0;
    }

    int MakeDirectoryInfo()
    {
        std::string strPath;
        //std::list<FILEINFO> lstFileInfos;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("命令解析错误！"));
            return -1;
        }
        if (_chdir(strPath.c_str()) != 0) {
            if (errno == EINVAL) {
                exit(0);
            }
            //_chdir
            FILEINFO finfo;
            //finfo.Isinvalid = TRUE;
            //finfo.IsDirectory = TRUE;//Q：问题，这里为什么是true
            //finfo.HasNext = FALSE;
            //memcpy(finfo.FileName, strPath.c_str(), strPath.size());
            //lstFileInfos.push_back(finfo);
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServerSocket::getInstance()->Send(pack);
            OutputDebugString(_T("没有权限访问目录\n"));
            return -2;
        }
        _finddata_t fdata;
        intptr_t hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            _findclose(hfind);
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServerSocket::getInstance()->Send(pack);
            OutputDebugString(_T("没有找到任何文件"));
            return -3;
        }
        int count = 0;
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.FileName, fdata.name, strlen(fdata.name));
            //lstFileInfos.push_back(finfo);
            TRACE("%s\r\n", finfo.FileName);
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServerSocket::getInstance()->Send(pack);
            count++;
        } while (!_findnext(hfind, &fdata));
        TRACE("server send file count:%d\r\n", count);
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        _findclose(hfind);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int RunFile()
    {
        std::string strPath;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("命令解析错误！"));
            return -1;
        }
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        CPacket pack(3, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int DownFile()
    {
        std::string strPath;
        long long data = 0;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("命令解析错误！"));
            return -1;
        }
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
        if (err != 0) {
            CPacket pack(4, (BYTE*)&data, 8);
            CServerSocket::getInstance()->Send(pack);
            return -2;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);
            //_ftelli64 获取文件指针当前未知
            data = _ftelli64(pFile);
            CPacket head(4, (BYTE*)&data, 8);
            //这里出现逻辑漏洞，之前未发现，直到做客户端对应功能才发觉
            CServerSocket::getInstance()->Send(head);
            fseek(pFile, 0, SEEK_SET);
            char buffer[1024] = "";
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                CPacket pack(4, (BYTE*)buffer, sizeof(buffer));
                CServerSocket::getInstance()->Send(pack);
            } while (rlen >= 1024);
            fclose(pFile);
        }
        CPacket pack(4, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int MoueEvent()
    {
        MOUSEEV mouse;
        if (CServerSocket::getInstance()->MoueEvent(mouse)) {

            int flags = 0;
            switch (mouse.nButton) {
            case 0://左键
                flags = 1;
                break;
            case 1://右键
                flags = 2;
                break;
            case 2://中建
                flags = 4;
                break;
            case 4://没有按键
                flags = 8;
                break;
            }
            if (flags != 8)SetCursorPos(mouse.pointXY.x, mouse.pointXY.y);
            switch (mouse.nAction) {
            case 0://单击
                flags |= 0x10;
                break;
            case 1://双击
                flags |= 0x20;
                break;
            case 2://按下
                flags |= 0x40;
                break;
            case 3://放开
                flags |= 0x80;
                break;
            default:
                break;
            }
            switch (flags) {
            case 0x21://左键双击
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x11:// 左键单击
                //mouse_event()模拟鼠标操作的API，GetMessageExtraInfo()获取键盘鼠标的额外信息
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x41://左键按下
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x81:// 左键放开
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x22://右键双击
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x12:// 右键单击
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x42://右键按下
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x82:// 右键放开
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x24: // 中键双击
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            case 0x14:// 中键单击
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x44: // 中键按下
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x84: //  中键放开
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x08: //单纯的移动
                mouse_event(MOUSEEVENTF_MOVE, mouse.pointXY.x, mouse.pointXY.y, 0, GetMessageExtraInfo());
                break;
            }
            CPacket pack(5, NULL, 0);
            CServerSocket::getInstance()->Send(pack);
        }
        else {
            OutputDebugString(_T("鼠标命令获取失败！"));
            return -1;
        }
        return 0;
    }

    int SendScreen()
    {
        CImage screen; //CImage 位图类
        HDC hScreen = ::GetDC(NULL);
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
        int nWitdth = GetDeviceCaps(hScreen, HORZRES);
        int nHeight = GetDeviceCaps(hScreen, VERTRES);
        screen.Create(nWitdth, nHeight, nBitPerPixel);
        BitBlt(screen.GetDC(), 0, 0, nWitdth, nHeight, hScreen, 0, 0, SRCCOPY);
        ReleaseDC(NULL, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//申请内存
        if (hMem == NULL)return -1;
        IStream* pStream = NULL;                     //流指针
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//将指针流变为内存流
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//重载函数，将数据变成流的形式
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);//重置指针到开头，才可读取完成数据
            PBYTE pData = (PBYTE)GlobalLock(hMem);   //获取数据
            SIZE_T nSize = GlobalSize(hMem);         //获取数据大小
            CPacket pack(6, pData, nSize);
            //TRACE("6:pack:%d\r\n", nSize);
            CServerSocket::getInstance()->Send(pack);
            GlobalUnlock(hMem);
        }
        //screen.Save(_T("test2022.png"), Gdiplus::ImageFormatPNG);
        //此调用将截图保存到了文件中,想要打包数据就还要从文件中取出再进行打包，所以利用Save重载，将其保存进内存中
        /* screen.Save(_T("test2022.BMP"), Gdiplus::ImageFormatBMP);*/
        /* for (int i = 0; i < 10; i++) {
             DWORD tick = (DWORD)GetTickCount64();
             screen.Save(_T("test2022.png"), Gdiplus::ImageFormatPNG);
             TRACE("png %d\r\n", (DWORD)GetTickCount64() - tick);
             tick = (DWORD)GetTickCount64();
             screen.Save(_T("test2022.jpg"), Gdiplus::ImageFormatJPEG);
             TRACE("jpg %d\r\n", (DWORD)GetTickCount64() - tick);
         }*/
        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();
        return 0;
    }
    static unsigned __stdcall threadLockDlg(void* arg)
    {
        CCommand* thiz = (CCommand*)arg;
        thiz->StartthreadLockDlg();
        _endthreadex(0);
        return 0;
    }

    void StartthreadLockDlg()
    {
        TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
        m_dlg.Create(IDD_DIALOG_INFO, NULL);
        m_dlg.ShowWindow(SW_SHOW);
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
        TRACE("right=%d bottom=%d\r\n", rect.right, rect.bottom);
        double(rect.bottom *= 1.20);
        m_dlg.MoveWindow(rect);
        CWnd* pText = m_dlg.GetDlgItem(IDC_STATIC_TALCK);
        if (pText) {
            CRect rtText;
            pText->GetWindowRect(rtText);
            int Width = rtText.Width();
            int x = (rect.right - Width) / 2;
            int nHeight = rtText.Height();
            int y = (rect.bottom - rtText.Height()) / 2;
            pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
        }
        m_dlg.SetWindowPos(&m_dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
        rect.left = 0;
        rect.top = 0;
        rect.right = 1;
        rect.bottom = 1;
        ClipCursor(rect);
        ShowCursor(false);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_KEYDOWN) {
                TRACE("msg:%08x wParam:%08x lParam:%08x\r\n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == 0x1B) break;
            }
        }
        ShowCursor(true);
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
        //_endthread();
        m_dlg.DestroyWindow();
    }

    int LockMachine()
    {
        if ((m_dlg.m_hWnd == NULL) || (m_dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
            //_beginthread(threadLockDlg, 0, NULL);
            _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &m_threadid);
            TRACE("threadid=%d\r\n", m_threadid);
        }
        CPacket pack(7, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }
    int UnLockMachine()
    {
        //dlg.SendMessage(WM_KEYDOWN, 0x1b, 0x00010001);
        //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1b, 0x00010001);
        //上面两个函数发送消息无效的原因是，windows消息机制，取消息的消息泵是依赖于当前线程的，所以无效
        //需要利用下面这个函数去发送消息
        PostThreadMessage(m_threadid, WM_KEYDOWN, 0x1B, 0);
        CPacket pack(8, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int DeleteLocalFile() {
        std::string strPath;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("命令解析错误！"));
            return -1;
        }
        //当对文件操作的时候需要考虑编码的问题
        TCHAR szPath[MAX_PATH] = _T("");
        //mbstowcs(szPath, strPath.c_str(), strPath.size());容易乱码
        MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size()
            , szPath, sizeof(szPath) / sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        CPacket pack(9, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }
    int TestLink()
    {
        CPacket pack(100, NULL, 0);
        bool ret = CServerSocket::getInstance()->Send(pack);
        if (ret == false) {
            TRACE("Test Send cmd is failed\r\n");
        }
        return 0;
    }
};

