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
/*ҵ���ܺ�������.h�¿���ʱ�滻���ģ�����ĺ��Ĺ��ܻ�����.cpp�У�����MFC����Ϣӳ��˼��*/
class CCommand
{
public:
	CCommand();
	~CCommand(){}
    int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket);
    static void RunCommand(void* arg, int status, 
        std::list<CPacket>& lstPacket, CPacket& inPacket) {
        CCommand* thiz = (CCommand*)arg;
        if (status > 0) {
            int ret = thiz->ExcuteCommand(status, lstPacket, inPacket);
            if (ret != 0) {
                TRACE("ִ������ʧ�ܣ�%d ret=%d\r\n", status, ret);
            }
        }
        else {
            MessageBox(NULL, _T("�޷����������û����Զ�����"), _T("�����û�ʧ�ܣ�"), MB_OK | MB_ICONERROR);
        }
    }
protected:
    CLockInFoDialg m_dlg;
    unsigned m_threadid;
	typedef int (CCommand::* FUNCCMD)(std::list<CPacket>&, CPacket& inPacket);//��Ա����ָ��
	std::map<int, FUNCCMD> m_mapFunc;//������ŵ����ܵ�ӳ��
protected:
    int MakeDirverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        /*
        int _chdrive(int drive);
        ���ã����ĵ�ǰ��������
        ������drive ָ����ǰ������������ 1 �� 26 ��������1=A��2=B �ȣ�
        ����ֵ������ѳɹ����ĵ�ǰ��������������Ϊ�� (0)������Ϊ -1
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
        //CTool::Dump((BYTE*)packet.Data(), packet.Size());
        lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }

    int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        //std::list<FILEINFO> lstFileInfos;
        if (_chdir(strPath.c_str()) != 0) {
            if (errno == EINVAL) {
                exit(0);
            }
            //_chdir
            FILEINFO finfo;
            //finfo.Isinvalid = TRUE;
            //finfo.IsDirectory = TRUE;//Q�����⣬����Ϊʲô��true
            //finfo.HasNext = FALSE;
            //memcpy(finfo.FileName, strPath.c_str(), strPath.size());
            //lstFileInfos.push_back(finfo);
            lstPacket.push_back(CPacket (2, (BYTE*)&finfo, sizeof(finfo)));
            OutputDebugString(_T("û��Ȩ�޷���Ŀ¼\n"));
            return -2;
        }
        _finddata_t fdata;
        intptr_t hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            _findclose(hfind);
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            OutputDebugString(_T("û���ҵ��κ��ļ�"));
            return -3;
        }
        int count = 0;
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.FileName, fdata.name, strlen(fdata.name));
            //lstFileInfos.push_back(finfo);
            TRACE("%s\r\n", finfo.FileName);
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            count++;
        } while (!_findnext(hfind, &fdata));
        TRACE("server send file count:%d\r\n", count);
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        _findclose(hfind);
        lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
        return 0;
    }

    int RunFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        lstPacket.push_back(CPacket(3, NULL, 0));
        return 0;
    }

    int DownFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        long long data = 0;
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
        if (err != 0) {
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            return -2;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);
            //_ftelli64 ��ȡ�ļ�ָ�뵱ǰδ֪
            data = _ftelli64(pFile);
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            //��������߼�©����֮ǰδ���֣�ֱ�����ͻ��˶�Ӧ���ܲŷ���
            fseek(pFile, 0, SEEK_SET);
            char buffer[1024] = "";
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                lstPacket.push_back(CPacket(4, (BYTE*)buffer, sizeof(buffer)));
            } while (rlen >= 1024);
            fclose(pFile);
        }
        lstPacket.push_back(CPacket(4, NULL, 0));
        return 0;
    }

    int MoueEvent(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        MOUSEEV mouse;
        memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEV));
        int flags = 0;
        switch (mouse.nButton) {
        case 0://���
            flags = 1;
            break;
        case 1://�Ҽ�
            flags = 2;
            break;
        case 2://�н�
            flags = 4;
            break;
        case 4://û�а���
            flags = 8;
            break;
        }
        if (flags != 8)SetCursorPos(mouse.pointXY.x, mouse.pointXY.y);
        switch (mouse.nAction) {
        case 0://����
            flags |= 0x10;
            break;
        case 1://˫��
            flags |= 0x20;
            break;
        case 2://����
            flags |= 0x40;
            break;
        case 3://�ſ�
            flags |= 0x80;
            break;
        default:
            break;
        }
        switch (flags) {
        case 0x21://���˫��
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11:// �������
            //mouse_event()ģ����������API��GetMessageExtraInfo()��ȡ�������Ķ�����Ϣ
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://�������
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81:// ����ſ�
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://�Ҽ�˫��
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12:// �Ҽ�����
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://�Ҽ�����
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82:// �Ҽ��ſ�
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24: // �м�˫��
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14:// �м�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44: // �м�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84: //  �м��ſ�
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08: //�������ƶ�
            mouse_event(MOUSEEVENTF_MOVE, mouse.pointXY.x, mouse.pointXY.y, 0, GetMessageExtraInfo());
            break;
        }
        lstPacket.push_back(CPacket(5, NULL, 0));
  
        OutputDebugString(_T("��������ȡʧ�ܣ�"));
        return -1;

        return 0;
    }

    int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        CImage screen; //CImage λͼ��
        HDC hScreen = ::GetDC(NULL);
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
        int nWitdth = GetDeviceCaps(hScreen, HORZRES);
        int nHeight = GetDeviceCaps(hScreen, VERTRES);
        screen.Create(nWitdth, nHeight, nBitPerPixel);
        BitBlt(screen.GetDC(), 0, 0, nWitdth, nHeight, hScreen, 0, 0, SRCCOPY);
        ReleaseDC(NULL, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//�����ڴ�
        if (hMem == NULL)return -1;
        IStream* pStream = NULL;                     //��ָ��
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//��ָ������Ϊ�ڴ���
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//���غ����������ݱ��������ʽ
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);//����ָ�뵽��ͷ���ſɶ�ȡ�������
            PBYTE pData = (PBYTE)GlobalLock(hMem);   //��ȡ����
            SIZE_T nSize = GlobalSize(hMem);  
            lstPacket.push_back(CPacket(6, pData, nSize));//��ȡ���ݴ�С
            GlobalUnlock(hMem);
        }
        //screen.Save(_T("test2022.png"), Gdiplus::ImageFormatPNG);
        //�˵��ý���ͼ���浽���ļ���,��Ҫ������ݾͻ�Ҫ���ļ���ȡ���ٽ��д������������Save���أ����䱣����ڴ���
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
        rect.bottom *= 1.20;
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

    int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        if ((m_dlg.m_hWnd == NULL) || (m_dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
            //_beginthread(threadLockDlg, 0, NULL);
            _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &m_threadid);
            TRACE("threadid=%d\r\n", m_threadid);
        }      
        lstPacket.push_back(CPacket(7, NULL, 0));
        return 0;
    }
    int UnLockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        //dlg.SendMessage(WM_KEYDOWN, 0x1b, 0x00010001);
        //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1b, 0x00010001);
        //������������������Ϣ��Ч��ԭ���ǣ�windows��Ϣ���ƣ�ȡ��Ϣ����Ϣ���������ڵ�ǰ�̵߳ģ�������Ч
        //��Ҫ���������������ȥ������Ϣ
        PostThreadMessage(m_threadid, WM_KEYDOWN, 0x1B, 0);
        lstPacket.push_back(CPacket(8, NULL, 0));
        return 0;
    }

    int DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        std::string strPath= inPacket.strData;
        //�����ļ�������ʱ����Ҫ���Ǳ��������
        TCHAR szPath[MAX_PATH] = _T("");
        //mbstowcs(szPath, strPath.c_str(), strPath.size());��������
        MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size()
            , szPath, sizeof(szPath) / sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        lstPacket.push_back(CPacket(9, NULL, 0));
        return 0;
    }
    int TestLink(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
       lstPacket.push_back(CPacket(100, NULL, 0));
        return 0;
    }
};

