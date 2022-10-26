﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "RemoteCtrl.h"
#include"ServerSocket.h"
#include<direct.h>
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
        OutputDebugString(_T("当前命令不是获取文件列表，命令解析错误！"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        //_chdir
        FILEINFO finfo;
        finfo.Isinvalid = TRUE;
        finfo.IsDirectory = TRUE;
        finfo.HasNext = FALSE;
        memcpy(finfo.FileName, strPath.c_str(), strPath.size());
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
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
            int nCmd = 1;
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
