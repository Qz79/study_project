// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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
    //CServerSocket::getInstance()->Send(packet);
    return 0;
}
#include<io.h>
#include<list>
typedef struct file_info {
    file_info() {
        Isinvalid = FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(FileName, 0, sizeof(FileName));
    }
    BOOL Isinvalid;
    BOOL IsDirectory;
    BOOL HasNext;
    char FileName[256];
}FILEINFO,*PFILEINFO;
int  MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO> lstFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("命令解析错误！"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        //_chdir
        FILEINFO finfo;
        finfo.Isinvalid = TRUE;
        finfo.IsDirectory = TRUE;//Q：问题，这里为什么是true
        finfo.HasNext = FALSE;
        memcpy(finfo.FileName, strPath.c_str(), strPath.size());
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;
    intptr_t hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    }
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.FileName,fdata.name,strlen(fdata.name));
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
    } while (!_findnext(hfind, &fdata));
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
int  LockMachine() {
    dlg.Create(IDD_DIALOG_INFO, NULL);
    dlg.ShowWindow(SW_SHOW);
    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (msg.message == WM_KEYDOWN) {
            TRACE("msg:%08x wParam:%08x lParam:%08x\r\n", msg.message, msg.wParam, msg.lParam);
            if (msg.wParam == 0x1B) break;
        }
    }
    dlg.DestroyWindow();
    return 0;
}
int UnLockMachine() {
    return 0;
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
        //    CServerSocket*pserver=CServerSocket::getInstance();
        //    if (pserver->InitSocket() == false) {
        //        MessageBox(NULL, _T("网络初始化失败，请检查网络"), _T("提示"), MB_OK | MB_ICONERROR);
        //        exit(0);
        //    }
        //    int count = 0;
        //    while (CServerSocket::getInstance() != NULL) {
        //        if (pserver->AcceptClient() == false) {
        //            if (count >= 3) {
        //                MessageBox(NULL, _T("多次重试无效，结束程序"), _T("提示"), MB_OK | MB_ICONERROR);
        //                exit(0);
        //            }
        //            MessageBox(NULL, _T("客户端连接异常,自动重试中"), _T("提示"), MB_OK | MB_ICONERROR);
        //            count++;
        //        }  
        //        int ret = pserver->DealCommand();//TODO:
        //    }
          
            int nCmd = 7;
            switch (nCmd) {
            case 1:
                MakeDirverInfo();
                break;
            case 2:
                MakeDirectoryInfo();
                break;
            case 3:
                RunFile();
                break;
            case 4:
                DownFile();
                break;
            case 5:
                MoueEvent();
                break;
            case 6:
                SendScreen();
                break;
            case 7:
                LockMachine();
                break;
            case 8:
                UnLockMachine();
                break;
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
