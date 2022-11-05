﻿
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_addr_server(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_addr_server);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
}

int CRemoteClientDlg::SendCmdPack(int nCmd, bool AutoClose,BYTE* pData, size_t nLength)
{
	UpdateData();
	CClinetSocket* pclient = CClinetSocket::getInstance();
	bool ret = pclient->InitSocket(m_addr_server, atoi(m_nPort));
	if (ret == false) {
		AfxMessageBox("初始化失败！");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	ret = pclient->Send(pack);
	if (ret == false) {
		TRACE("Client Test Send cmd is failed\r\n");
		return -2;
	}
	int cmd=pclient->DealCommand();
	TRACE("recv the cmd is:%d\r\n", pclient->GetPacket().sCmd);
	if (AutoClose)
		pclient->CloseCliSocket();
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	/*UpdateData() 是MFC的窗口函数，用来刷新数据的。
	总的来说：操作系统会调用这个函数来初始化对话框中的数据，或者检索或者验证对话框中的数据。
	UpdateData(true);//用于将屏幕上控件中的数据交换到变量中。
	UpdateData(false);//用于将数据在屏幕中对应控件中显示出来。
	当你使用了ClassWizard建立了控件和变量之间的联系后：当你修改了变量的值，
	而希望对话框控件更新显示，就应该在修改变量后调用UpdateData(FALSE)；
	如果你希望知道用户在对话框中到底输入了什么，就应该在访问变量前调用UpdateData(TRUE)。*/
	m_addr_server = 0x7F000001;
	m_nPort = "9527";
	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteClientDlg::OnBnClickedBtnTest()
{
	int ret=SendCmdPack(100);
	if (ret == -1 || ret == -2) {
		AfxMessageBox("发送命令失败！");
		return;
	}
}


void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = SendCmdPack(1);
	if (ret == -1 || ret == -2) {
		AfxMessageBox("发送命令失败！");
		return;
	}
	CClinetSocket* pclient = CClinetSocket::getInstance();
	std::string drives = pclient->GetPacket().strData;
	std::string dr;
	m_Tree.DeleteAllItems();
	for (size_t i = 0; i < drives.size(); i++) {
		if (drives [i]== ',') {
			dr += ":";
			HTREEITEM Temp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			m_Tree.InsertItem(NULL, Temp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drives[i];
	}
}
CString CRemoteClientDlg::GetPath(HTREEITEM hTree) {
	CString strRet, strTemp;
	do {
		strTemp = m_Tree.GetItemText(hTree);
		strRet = strTemp + "//" + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChildItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != NULL)m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint point;
	GetCursorPos(&point);
	m_Tree.ScreenToClient(&point);//这里的转换点击坐标理解上还是存在误区
	HTREEITEM hTreeItem = m_Tree.HitTest(point, 0);
	if (hTreeItem == NULL)
		return;
	if (m_Tree.GetChildItem(hTreeItem) == NULL)
		return;
	DeleteTreeChildItem(hTreeItem);//防止多次双击产生多次添加
	CString strPath = GetPath(hTreeItem);
	int cmd = SendCmdPack(2,false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	CClinetSocket* pclient = CClinetSocket::getInstance();
	PFILEINFO pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
	while (pfile->HasNext) {
		TRACE("[%s] isdir:%d\r\n", pfile->FileName, pfile->IsDirectory);
		if (pfile->IsDirectory) {
			if ((CString(pfile->FileName) == ".") || (CString(pfile->FileName) == "..")) {
				cmd = pclient->DealCommand();
				TRACE("recv the cmd is:%d\r\n", pclient->GetPacket().sCmd);
				if (cmd < 0)break;
				pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
				continue;
			}
		}
		HTREEITEM Temp=m_Tree.InsertItem(pfile->FileName,hTreeItem,TVI_LAST);
		if (pfile->IsDirectory) {
			m_Tree.InsertItem(NULL, Temp, TVI_LAST);
		}
		int cmd = pclient->DealCommand();
		TRACE("recv the cmd is:%d\r\n", pclient->GetPacket().sCmd);
		if (cmd < 0)break;
		pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
	}
	pclient->CloseCliSocket();
}
