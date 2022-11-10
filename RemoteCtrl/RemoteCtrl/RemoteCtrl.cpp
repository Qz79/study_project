﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include"ServerSocket.h"
#include<direct.h>
#include<atlimage.h>
#include"CLockInFoDialg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 入口函数改变的代码形式：
//#pragma comment(linker,"/subsystem:windows/entry:WinMainCTRStartup")
//#pragma comment(linker,"/subsystem:windows/entry:mainCTRStartup")
//#pragma comment(linker,"/subsystem:console/entry:WinMainCTRStartup")
//#pragma comment(linker,"/subsystem:console/entry:mainCTRStartup")

// 唯一的应用程序对象
// 001测试分支切换演示
CWinApp theApp;

using namespace std;
void Dump(BYTE* pData, size_t nSize) {
    std::string strOut;
    for (size_t i = 0; i < nSize; i++) {
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0))strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);//&0xFF的操作是为了防止符号位产生负数
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}
int MakeDirverInfo() {
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
    Dump((BYTE*)packet.Data(), packet.Size());
    CServerSocket::getInstance()->Send(packet);
    return 0;
}
#include<io.h>
#include<list>

int  MakeDirectoryInfo() {
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
        memcpy(finfo.FileName,fdata.name,strlen(fdata.name));
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
int RunFile() {
    std::string strPath;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("命令解析错误！"));
        return -1;
    }
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    CPacket pack(3, NULL,0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}
int DownFile() {
    std::string strPath;
    long long data = 0;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("命令解析错误！"));
        return -1;
    }
    FILE* pFile = NULL;
    errno_t err=fopen_s(&pFile,strPath.c_str(), "rb");
    if (err != 0) {
        CPacket pack(4,(BYTE*)&data, 8);
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
int MoueEvent() {
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
        if(flags!=8)SetCursorPos(mouse.pointXY.x, mouse.pointXY.y);
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
int SendScreen() {
    CImage screen; //CImage 位图类
    HDC hScreen = ::GetDC(NULL); 
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
    int nWitdth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWitdth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, 1366, 748, hScreen, 0, 0, SRCCOPY);
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
CLockInFoDialg dlg;
unsigned threadid;
//void threadLockDlg(void* arg);
unsigned __stdcall threadLockDlg(void* arg) {
    TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
    dlg.Create(IDD_DIALOG_INFO, NULL);
    dlg.ShowWindow(SW_SHOW);
    CRect rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    TRACE("right=%d bottom=%d\r\n", rect.right, rect.bottom);
    double(rect.bottom *= 1.10);
    dlg.MoveWindow(rect);
    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
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
    dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}
int  LockMachine() {
    if ((dlg.m_hWnd == NULL) ||( dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
        //_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);
        TRACE("threadid=%d\r\n", threadid);
    }
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}
int UnLockMachine() {
    //dlg.SendMessage(WM_KEYDOWN, 0x1b, 0x00010001);
    //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1b, 0x00010001);
    //上面两个函数发送消息无效的原因是，windows消息机制，取消息的消息泵是依赖于当前线程的，所以无效
    //需要利用下面这个函数去发送消息
    PostThreadMessage(threadid, WM_KEYDOWN, 0x1B,0);
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
int TestLink() {
    CPacket pack(100, NULL, 0);
    bool ret=CServerSocket::getInstance()->Send(pack);
    if (ret == false) {
        TRACE("Test Send cmd is failed\r\n");
    }
    return 0;
}
int ExcuteCommand(int nCmd) {
    int ret = 0;
    switch (nCmd) {
    case 1:
        ret = MakeDirverInfo();
        break;
    case 2:
        ret = MakeDirectoryInfo();
        break;
    case 3:
        ret = RunFile();
        break;
    case 4:
        ret = DownFile();
        break;
    case 5:
        ret = MoueEvent();
        break;
    case 6:
        ret = SendScreen();
        break;
    case 7:
        ret = LockMachine();
        /*Sleep(50);
        LockMachine();*///T0:测试多次调用线程会不会产生问题
        break;
    case 8:
        ret = UnLockMachine();
        break;
    case 9:
        ret = DeleteLocalFile();
    case 100:
        ret = TestLink();
        break;
    }
    /*T1:测试解锁
    UnLockMachine();
    TRACE("m_hWnd=%08x\r\n", dlg.m_hWnd);
    while (dlg.m_hWnd != NULL) {
        Sleep(10);
    }*/
    /*while ((dlg.m_hWnd != NULL)&&(dlg.m_hWnd != INVALID_HANDLE_VALUE))
        Sleep(100);
      T0: 测试多次调用线程会不会产生问题
    */
    return ret;
}
int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
        //    // TODO: 在此处为应用程序的行为编写代码。
        //    // socket、bind、liten、accpet、read、write、close
        //    // 初始化环境，Windows下面有一个环境的初始化的，用到WSADATA
            CServerSocket*pserver=CServerSocket::getInstance();
            if (pserver->InitSocket() == false) {
                MessageBox(NULL, _T("网络初始化失败，请检查网络"), _T("提示"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            int count = 0;
            while (CServerSocket::getInstance() != NULL) {
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        MessageBox(NULL, _T("多次重试无效，结束程序"), _T("提示"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T("客户端连接异常,自动重试中"), _T("提示"), MB_OK | MB_ICONERROR);
                    count++;
                }  
                int ret = pserver->DealCommand();
                if (ret > 0) {
                    ret = ExcuteCommand(pserver->GetPacket().sCmd);
                    if (ret != 0) {
                        TRACE("执行命令失败：%d,ret=%d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    pserver->CloseCliSocket();
                }
            }           
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
