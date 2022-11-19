// WatchDlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "WatchDlg.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"


// CWatchDlg 对话框

IMPLEMENT_DYNAMIC(CWatchDlg, CDialog)

CWatchDlg::CWatchDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_nObjWidth = -1;
	m_nObjHeight = -1;
}

CWatchDlg::~CWatchDlg()
{
}

void CWatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_STATIC_PICTURE, &CWatchDlg::OnStnClickedStaticPicture)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDlg::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDlg::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDlg 消息处理程序


CPoint CWatchDlg::UserPoint2RemoteScreenP(CPoint& point, bool isScreen)
{
	//这里的坐标转换不够完善，要先获取监控端的大小比例，再去做运算
	if (isScreen)ScreenToClient(&point);
	CRect ClinetRect;
	m_picture.GetWindowRect(ClinetRect);
	return CPoint((point.x * m_nObjWidth / ClinetRect.Width()), (point.y * m_nObjHeight / ClinetRect.Height()));
}

BOOL CWatchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO:  在此添加额外的初始化
	SetTimer(0, 50, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull()) {
			//TODO：做显示处理
			/*
			首先显示就应该想到DC，在MFC中CDC的类就是负责，文字，图像，绘制，背景，这一系列都与显示有关
			所以碰到显示首先就应该想到设备上下文DC
			*/
			//BitBLt()对位图的位进行操作
			CRect rect;
			m_picture.GetWindowRect(rect);
			if (m_nObjWidth == -1) {
				m_nObjWidth = pParent->GetImage().GetWidth();
			}
			if (m_nObjHeight == -1) {
				m_nObjHeight = pParent->GetImage().GetHeight();
			}
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			//这里依旧需要优化，监控端的画面比例需要获取，再去适配客户端大小，当服务端大小改变时，图像理应随之改变
			pParent->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(),
				0, 0, rect.Width(), rect.Height(), SRCCOPY);//对位图的缩放
		
			CClientDC ClientDC(this);
			ClientDC.SetWindowExt(m_nObjWidth, m_nObjHeight);
			m_picture.InvalidateRect(NULL);
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if((m_nObjWidth!=-1)&& (m_nObjHeight!=-1)){
		CPoint remot = UserPoint2RemoteScreenP(point);
		MOUSEEV event; 
		event.nButton = 0;
		event.nAction = 1;
		event.pointXY = remot;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remot = UserPoint2RemoteScreenP(point);
		MOUSEEV event;
		event.nButton = 0;
		event.nAction = 2;
		event.pointXY = remot;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remot = UserPoint2RemoteScreenP(point);
		MOUSEEV event;
		event.nButton = 0;
		event.nAction = 3;
		event.pointXY = remot;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remot = UserPoint2RemoteScreenP(point);
		MOUSEEV event;
		event.nButton = 1;
		event.nAction = 1;
		event.pointXY = remot;
		//存在隐患，网络通信和对话框存在耦合
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remot = UserPoint2RemoteScreenP(point);
		MOUSEEV event;
		event.nButton = 1;
		event.nAction = 2;
		event.pointXY = remot;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
    CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remot = UserPoint2RemoteScreenP(point);
		MOUSEEV event;
		event.nButton = 1;
		event.nAction = 3;
		event.pointXY = remot;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint remot = UserPoint2RemoteScreenP(point);
		MOUSEEV event;
		event.nButton = 4;
		event.nAction = 8;
		event.pointXY = remot;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDlg::OnStnClickedStaticPicture()
{
	// TODO: 在此添加控件通知处理程序代码
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint point;
		GetCursorPos(&point);
		CPoint remot = UserPoint2RemoteScreenP(point, true);
		MOUSEEV event;
		event.nButton = 0;
		event.nAction = 0;
		event.pointXY = remot;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
}


void CWatchDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}


void CWatchDlg::OnBnClickedBtnLock()
{
	// TODO: 在此添加控件通知处理程序代码
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}


void CWatchDlg::OnBnClickedBtnUnlock()
{
	// TODO: 在此添加控件通知处理程序代码
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1); 
}
