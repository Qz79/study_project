
// RemoteClientDlg.h: 头文件
//

#pragma once

#include"ClinetSocket.h"
#include"DlgStatus.h"

#define WM_SEND_PACKET (WM_USER+1)
// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


private:
	int SendCmdPack(int nCmd, bool AutoClose=true, BYTE* pData=NULL, size_t nLength=0);
public:
	bool isFull()const {
		return m_isFull;
	}
	CImage& GetImage() {
		return m_image;
	}
	void SetImageStatus(bool isFull=false) {
		m_isFull = isFull;
	}
private:
	CImage m_image;
	bool m_isFull;
	bool m_isClose;
	// 实现
protected:
	HICON m_hIcon;
	CDlgStatus m_dlgStatus;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_addr_server;
	CString m_nPort;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
private:
	CString CRemoteClientDlg::GetPath(HTREEITEM hTree);
	void DeleteTreeChildItem(HTREEITEM hTree);
	void LoadFileInfo();
	void LoadFileCurrent();
	static void threadEntryForDownFile(void* arg);
	void threadDownFile();
	static void threadEntryForWatch(void* arg);//静态函数没有this指针
	void threadWatch();
public:
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRunFile();
	afx_msg void OnDownFile();
	afx_msg void OnDeleteFile();
	afx_msg LRESULT SendPack(WPARAM wParam,LPARAM lParam);
	afx_msg void OnBnClickedBtnWatch();
	//afx_msg void OnTimer(UINT_PTR nIDEvent);
};
