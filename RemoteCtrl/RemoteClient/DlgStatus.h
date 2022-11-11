﻿#pragma once


// CDlgStatus 对话框

class CDlgStatus : public CDialog
{
	DECLARE_DYNAMIC(CDlgStatus)

public:
	CDlgStatus(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDlgStatus();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_STATUS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditStatus;
};
