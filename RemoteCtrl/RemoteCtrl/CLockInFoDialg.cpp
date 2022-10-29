// CLockInFoDialg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "CLockInFoDialg.h"
#include "afxdialogex.h"


// CLockInFoDialg 对话框

IMPLEMENT_DYNAMIC(CLockInFoDialg, CDialog)

CLockInFoDialg::CLockInFoDialg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

CLockInFoDialg::~CLockInFoDialg()
{
}

void CLockInFoDialg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLockInFoDialg, CDialog)
END_MESSAGE_MAP()


// CLockInFoDialg 消息处理程序
