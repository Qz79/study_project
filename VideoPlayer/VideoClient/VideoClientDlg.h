
// VideoClientDlg.h: 头文件
//

#pragma once


// CVideoClientDlg 对话框
class CVideoClientDlg : public CDialogEx
{
// 构造
public:
	CVideoClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEOCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	bool m_status_btnPlay; //标记播放按钮状态bool变量
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CEdit m_video;         //视频显示框的变量
	CSliderCtrl m_position;//播放滑动条变量
	CSliderCtrl m_volume;  //音量滑动条变量
	CEdit m_url;           //播放地址显示框变量
	CButton m_btnPlay;     //播放按钮变量
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnStop();


	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

};
