// DlgStatus.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "DlgStatus.h"
#include "afxdialogex.h"


// CDlgStatus 对话框

IMPLEMENT_DYNAMIC(CDlgStatus, CDialog)

CDlgStatus::CDlgStatus(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_STATUS, pParent)
{

}

CDlgStatus::~CDlgStatus()
{
}

void CDlgStatus::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_EditStatus);
}


BEGIN_MESSAGE_MAP(CDlgStatus, CDialog)
END_MESSAGE_MAP()


// CDlgStatus 消息处理程序
