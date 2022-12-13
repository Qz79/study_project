
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "ClinetController.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "WatchDlg.h"



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
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}


BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	//ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	ON_COMMAND(ID_DOWN_FILE, &CRemoteClientDlg::OnDownFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::SendPack)
	ON_BN_CLICKED(IDC_BTN_WATCH, &CRemoteClientDlg::OnBnClickedBtnWatch)
	//ON_WM_TIMER()
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddressServ)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CRemoteClientDlg::OnEnChangeEditPort)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CRemoteClientDlg::OnSendPackAck)
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
	InitUIData();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::DealCommand(WORD nCmd, const std::string& strData, LPARAM lParam)
{
	switch (nCmd) {
	case 1://获取驱动信息
		Str2Tree(strData, m_Tree);
		break;
	case 2://获取文件信息
		UpdateFileInfo(*(PFILEINFO)strData.c_str(), (HTREEITEM)lParam);
		break;
	case 3:
		MessageBox("打开文件完成！", "操作完成", MB_ICONINFORMATION);
		break;
	case 4:
		UpdateDownloadFile(strData, (FILE*)lParam);
		break;
	case 9:
		MessageBox("删除文件完成！", "操作完成", MB_ICONINFORMATION);
		break;
	case 100:
		MessageBox("连接测试成功！", "连接成功", MB_ICONINFORMATION);
		break;
	default:
		TRACE("unknow data received! %d\r\n", nCmd);
		break;
	}
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
	int ret= CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 100);
	if (ret == -1 || ret == -2) {
		AfxMessageBox("发送命令失败！");
		return;
	}
}


void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 1, true, NULL, 0);
	if (ret == -1 || ret == -2) {
		AfxMessageBox("发送命令失败！");
		return;
	}
}
void CRemoteClientDlg::Str2Tree(const std::string& drivers, CTreeCtrl& tree)
{
	std::string dr;
	tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',') {
			dr += ":";
			HTREEITEM hTemp = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			tree.InsertItem("", hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	if (dr.size() > 0) {
		dr += ":";
		HTREEITEM hTemp = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
		tree.InsertItem("", hTemp, TVI_LAST);
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

void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL)
		return;
	DeleteTreeChildItem(hTreeSelected);
	m_List.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);
	TRACE("hTreeSelected %08X\r\n", hTreeSelected);
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(), (WPARAM)hTreeSelected);
}
//	PFILEINFO pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
//	int count = 0;
//	while (pfile->HasNext) {
//		TRACE("[%s] isdir:%d\r\n", pfile->FileName, pfile->IsDirectory);
//		if (pfile->IsDirectory) {
//			if ((CString(pfile->FileName) == ".") || (CString(pfile->FileName) == "..")) {
//				cmd = pclient->DealCommand();
//				TRACE("recv the cmd is:%d\r\n", pclient->GetPacket().sCmd);
//				if (cmd < 0)break;
//				pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
//				continue;
//			}
//			HTREEITEM Temp = m_Tree.InsertItem(pfile->FileName, hTreeItem, TVI_LAST);
//			m_Tree.InsertItem("", Temp, TVI_LAST);
//		}
//		else {
//			m_List.InsertItem(0, pfile->FileName);
//		}
//		
//		int cmd = pclient->DealCommand();
//		TRACE("recv the cmd is:%d\r\n", pclient->GetPacket().sCmd);
//		if (cmd < 0)break;
//		pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
//		count++;
//	}
//	TRACE("client recv file count:%d\r\n", count);
//	pclient->CloseCliSocket();
//}

void CRemoteClientDlg::LoadFileCurrent()
{
	HTREEITEM hTreeItem = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTreeItem);
	m_List.DeleteAllItems();
	int cmd = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	CClinetSocket* pclient = CClinetSocket::getInstance();
	PFILEINFO pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
	while (pfile->HasNext) {
		TRACE("[%s] isdir:%d\r\n", pfile->FileName, pfile->IsDirectory);
		if (!pfile->IsDirectory) {
			m_List.InsertItem(0, pfile->FileName);
		}

		int cmd = pclient->DealCommand();
		TRACE("recv the cmd is:%d\r\n", pclient->GetPacket().sCmd);
		if (cmd < 0)break;
		pfile = (PFILEINFO)pclient->GetPacket().strData.c_str();
	}
	pclient->CloseCliSocket();
	//TODO:大文件传输需要额外处理
}

void CRemoteClientDlg::UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent)
{
	TRACE("hasnext %d isdirectory %d %s\r\n", finfo.HasNext, finfo.IsDirectory, finfo.FileName);
	if (finfo.HasNext == FALSE)return;
	if (finfo.IsDirectory) {
		if (CString(finfo.FileName) == "." || (CString(finfo.FileName) == ".."))
			return;
		TRACE("hselected %08X %08X\r\n", hParent, m_Tree.GetSelectedItem());
		HTREEITEM hTemp = m_Tree.InsertItem(finfo.FileName, hParent);
		m_Tree.InsertItem("", hTemp, TVI_LAST);
		m_Tree.Expand(hParent, TVE_EXPAND);
	}
	else {
		m_List.InsertItem(0, finfo.FileName);
	}
}

void CRemoteClientDlg::UpdateDownloadFile(const std::string& strData, FILE* pFile)
{
	static LONGLONG length = 0, index = 0;
	TRACE("length %d index %d\r\n", length, index);
	if (length == 0) {
		length = *(long long*)strData.c_str();
		if (length == 0) {
			AfxMessageBox("文件长度为零或者无法读取文件！！！");
			CClientController::getInstance()->DownloadEnd();
		}
	}
	else if (length > 0 && (index >= length)) {
		fclose(pFile);
		length = 0;
		index = 0;
		CClientController::getInstance()->DownloadEnd();
	}
	else {
		fwrite(strData.c_str(), 1, strData.size(), pFile);
		index += strData.size();
		TRACE("index = %d\r\n", index);
		if (index >= length) {
			fclose(pFile);
			length = 0;
			index = 0;
			CClientController::getInstance()->DownloadEnd();
		}
	}
}

void CRemoteClientDlg::InitUIData()
{
	UpdateData();
	/*UpdateData() 是MFC的窗口函数，用来刷新数据的。
	总的来说：操作系统会调用这个函数来初始化对话框中的数据，或者检索或者验证对话框中的数据。
	UpdateData(true);//用于将屏幕上控件中的数据交换到变量中。
	UpdateData(false);//用于将数据在屏幕中对应控件中显示出来。
	当你使用了ClassWizard建立了控件和变量之间的联系后：当你修改了变量的值，
	而希望对话框控件更新显示，就应该在修改变量后调用UpdateData(FALSE)；
	如果你希望知道用户在对话框中到底输入了什么，就应该在访问变量前调用UpdateData(TRUE)。*/
	m_addr_server = 0xC0A87387; //192.168.115.135
	m_nPort = "9527";
	CClientController::getInstance()->
		UpdateAddr(m_addr_server, atoi((LPCTSTR)m_nPort));
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
}


LRESULT CRemoteClientDlg::OnSendPackAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || (lParam == -2)) {
		TRACE("socket is error %d\r\n", lParam);
	}
	else if (lParam == 1) {
		//对方关闭了套接字
		TRACE("socket is closed!\r\n");
	}
	else {
		if (wParam != NULL) {
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			DealCommand(head.sCmd, head.strData, lParam);
		}
	}
	return 0;
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	// Q1：单击事件未响应hTreeItem 值为空，坐标转换后为负数？
	// Q3：当反复单击的时候，加载文件数量不一样，又是哪里的问题
	
	*pResult = 0;
	LoadFileInfo();
	/* Q2：当双击事件取消时，依旧需要双击才会显示，
		  响应了单击事件，文件信息也都传过来，但是图形化界面没有显示问题？*/
	//A2: 第二个问题解决，使用为本身属性就没设置单个展开
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}





void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint pMouse,pList;
	GetCursorPos(&pMouse);
	pList = pMouse;
	m_List.ScreenToClient(&pList);
	int ListSelected = m_List.HitTest(pList);
	if (ListSelected < 0)return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPopup = menu.GetSubMenu(0);
	if (pPopup != NULL) {
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pMouse.x, pMouse.y, this);
	}

}


void CRemoteClientDlg::OnRunFile()
{
	int SelectedList = m_List.GetSelectionMark();
	CString strPath = m_List.GetItemText(SelectedList, 0);
	HTREEITEM hTreeSelect = m_Tree.GetSelectedItem();
	strPath = GetPath(hTreeSelect) + strPath;
	TRACE("OpenFile strPath:%s\r\n", strPath);
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 3, true, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	if (ret < 0) {
		AfxMessageBox(_T("发送命令失败！"));
		TRACE("send three cmd failed:%d\r\n", ret);
		return;
	}
}


void CRemoteClientDlg::OnDownFile()
{
	int SelectedList = m_List.GetSelectionMark();
	CString strPath = m_List.GetItemText(SelectedList, 0);
	int ret=CClientController::getInstance()->DownFile(strPath);
}


void CRemoteClientDlg::OnDeleteFile()
{
	int SelectedList = m_List.GetSelectionMark();
	CString strPath = m_List.GetItemText(SelectedList, 0);
	HTREEITEM hTreeSelect = m_Tree.GetSelectedItem();
	strPath = GetPath(hTreeSelect) + strPath;
	TRACE("OpenFile strPath:%s\r\n", strPath);
	CClinetSocket* pclient = CClinetSocket::getInstance();
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 9, true, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	if (ret < 0) {
		AfxMessageBox(_T("发送命令失败！"));
		TRACE("send three cmd failed:%d\r\n", ret);
		return;
	}
	/*if (pclient->GetPacket().sCmd == 9)
		m_List.UpdateWindow();远程控制所以需要重新获取数据才才算刷新，光调用接口无用*/
	LoadFileCurrent();
}

LRESULT CRemoteClientDlg::SendPack(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || (lParam == -2)) {
		TRACE("socket is error %d\r\n", lParam);
	}
	else if (lParam == 1) {
		//对方关闭了套接字
		TRACE("socket is closed!\r\n");
	}
	else {
		if (wParam != NULL) {
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			DealCommand(head.sCmd, head.strData, lParam);
		}
	}
	return 0;
}


void CRemoteClientDlg::OnBnClickedBtnWatch()
{
	// TODO: 在此添加控件通知处理程序代码
	CClientController::getInstance()->StratWatchcreen();
}

void CRemoteClientDlg::OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CClientController::getInstance()->UpdateAddr(m_addr_server, atoi((LPCTSTR)m_nPort));
	*pResult = 0;
}


void CRemoteClientDlg::OnEnChangeEditPort()
{
	UpdateData();
	CClientController::getInstance()->UpdateAddr(m_addr_server, atoi((LPCTSTR)m_nPort));
}
