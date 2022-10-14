﻿
// VideoClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "VideoClient.h"
#include "VideoClientDlg.h"
#include "afxdialogex.h"
#include"ClientController.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVideoClientDlg 对话框



CVideoClientDlg::CVideoClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VIDEOCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_status_btnPlay = false;
	m_length = 0.0f;
}

void CVideoClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_VIDEO, m_video);
	DDX_Control(pDX, IDC_SLIDER_POS, m_position);
	DDX_Control(pDX, IDC_SLIDER_VOL, m_volume);
	DDX_Control(pDX, IDC_EDIT_URL, m_url);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
}

BEGIN_MESSAGE_MAP(CVideoClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CVideoClientDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, &CVideoClientDlg::OnBnClickedBtnStop)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CVideoClientDlg 消息处理程序

BOOL CVideoClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	SetTimer(0, 500, NULL);
	m_position.SetRange(0, 1);
	m_volume.SetRange(0, 100);
	m_position.SetTicFreq(20);
	SetDlgItemText(IDC_STATIC_VOL, _T("100%"));
	SetDlgItemText(IDC_STATIC_TIME, _T("--:--:--/--:--:--"));
	m_controller->SetHwnd(m_video.GetSafeHwnd());
	m_url.SetWindowText(_T("file:///F:\\Users\\NING MEI\\source\\study_project\\VideoPlayer\\VideoClient\\远程控制.mp4"));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVideoClientDlg::OnPaint()
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
HCURSOR CVideoClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVideoClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		//Controller获取播放状态以及进度信息

		float pos = m_controller->VideoCtrl(VLC_GET_POSITION);
		TRACE("pos:%f\r\n", pos);
		if (pos != -1.0f) {
			if (m_length <= 0.0f)
				m_length=m_controller->VideoCtrl(VLC_GET_LENGTH);
			//m_position的长度需要判断设置
			TRACE("length:%f\r\n", m_length);
			if (m_position.GetRangeMax() <= 1) {
				m_position.SetRange(0, int(m_length));
			}
            CString strPos;
			//这里需要学习Format的用法
			strPos.Format(_T("%f/%f"), pos*m_length, m_length);
			//这里需要了解SetDlgItemText的用法
			SetDlgItemText(IDC_STATIC_TIME, strPos);
			m_position.SetPos(int(m_length *pos));
		}
		//IDC_STATIC_VOL更新处理音量
		//IDC_STATIC_TIME 更新播放时间
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CVideoClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(0);
	// TODO: 在此处添加消息处理程序代码
}





void CVideoClientDlg::OnBnClickedBtnPlay()
{
	if (m_status_btnPlay == false) {
		
		CString url;
		m_url.GetWindowText(url);
		//判断一下当前是否为恢复
		m_controller->SetMedia(m_controller->Unicode2Utf8((LPCTSTR)url));
		m_btnPlay.SetWindowText(_T("暂停"));
		m_status_btnPlay = true;
		//TODO:Controller的播放接口
		m_controller->VideoCtrl(VLC_PLAY);
	}
	else {
		m_btnPlay.SetWindowText(_T("播放"));
		m_status_btnPlay = false;
		//TODO:Controller的暂停接口
		m_controller->VideoCtrl(VLC_PAUSE);
	}
}


void CVideoClientDlg::OnBnClickedBtnStop()
{
	m_btnPlay.SetWindowText(_T("播放"));
	m_status_btnPlay = false;
	//TODO:Controller的停止接口
	m_controller->VideoCtrl(VLC_STOP);
}


void CVideoClientDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nSBCode == 5) {
		TRACE("pos %p volume %p cur %p pos %d\r\n", &m_position, &m_volume, pScrollBar, nPos);
		CString strPostion;
		strPostion.Format(_T("%d%%"),  nPos);
		SetDlgItemText(IDC_STATIC_TIME, strPostion);
		m_controller->SetPosition(float(nPos)/m_length);
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CVideoClientDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nSBCode == 5) {
		TRACE("pos %p volume %p cur %p pos %d\r\n", &m_position, &m_volume, pScrollBar, nPos);
		CString strVolume;
		strVolume.Format(_T("%d%%"), 100-nPos);
		SetDlgItemText(IDC_STATIC_VOL, strVolume);
		m_controller->SetVolume(100 - nPos);
	}
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}



