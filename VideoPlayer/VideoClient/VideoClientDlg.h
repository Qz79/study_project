
// VideoClientDlg.h: 头文件
//

#pragma once

class ClientController;

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
	
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	bool m_status_btnPlay; //标记播放按钮状态bool变量
	float m_length;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CEdit m_video;         //视频显示框的实例
	CSliderCtrl m_position;//播放滑动条实例
	CSliderCtrl m_volume;  //音量滑动条实例
	CEdit m_url;           //播放地址显示框实例
	CButton m_btnPlay;     //播放按钮实例
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
public:
	ClientController* m_controller;
};
