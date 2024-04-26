// OpenProcessDlg.cpp: 实现文件
//

#include "pch.h"
#include "SamSung Dive.h"
#include "afxdialogex.h"
#include "OpenProcessDlg.h"


// COpenProcessDlg 对话框

IMPLEMENT_DYNAMIC(COpenProcessDlg, CDialogEx)

COpenProcessDlg::COpenProcessDlg(CWnd* pParent, CIocpServer*
	IocpServer, CONTEXT_OBJECT* ContextObject)
	: CDialogEx(IDD_PROCESS_MANAGER_DIALOG, pParent)
{

	m_IocpServer = IocpServer;
	m_ContextObject = ContextObject;

	

}

COpenProcessDlg::~COpenProcessDlg()
{

}

void COpenProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OPEN_PROCESS_STATIC1, m_Description_Static);
	DDX_Control(pDX, IDOK, m_OK_Button);
	
}


BEGIN_MESSAGE_MAP(COpenProcessDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &COpenProcessDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_CONTROL, &COpenProcessDlg::OnEnChangeEditControl)
END_MESSAGE_MAP()


// COpenProcessDlg 消息处理程序


BOOL COpenProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_OK_Button.EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}




void COpenProcessDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CEdit* edit;
	edit = (CEdit*)GetDlgItem(IDC_EDIT_CONTROL);
	CString v1;
	edit->GetWindowText(v1);
	int BufferLength = sizeof(v1) + 1;
	char* BufferData = (char*)VirtualAlloc(NULL, BufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memset(BufferData, 0, sizeof(char)*BufferLength);
	sprintf(BufferData, "%s", v1.GetBuffer(0));
	m_IocpServer->OnPrepareSending(m_ContextObject,(PBYTE)BufferData,strlen(BufferData));



	CDialogEx::OnOK();
}


void COpenProcessDlg::OnEnChangeEditControl()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	m_OK_Button.EnableWindow(TRUE);



}
