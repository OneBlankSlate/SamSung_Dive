// ServerManagerDlg.cpp: 实现文件
//

#include "pch.h"
#include "SamSung Dive.h"
#include "afxdialogex.h"
#include "ServerManagerDlg.h"
#include"SamSung DiveDlg.h"

// CServerManagerDlg 对话框

IMPLEMENT_DYNAMIC(CServerManagerDlg, CDialogEx)

CServerManagerDlg::CServerManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_MANAGER_DIALOG, pParent)
	, m_SetListenPort(0)
	, m_SetMaxConnection(0)
{
	m_ConfigFile=((CSamSungDiveDlg*)pParent)->m_ConfigFile;
}

CServerManagerDlg::~CServerManagerDlg()
{
}

void CServerManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LISTENPORT_SETTING, m_SetListenPort);
	DDX_Text(pDX, IDC_MAX_CONNECTION_SETTING2, m_SetMaxConnection);
	DDX_Control(pDX, IDC_APPLY_BUTTON, m_ApplyButton);
}


BEGIN_MESSAGE_MAP(CServerManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_APPLY_BUTTON, &CServerManagerDlg::OnBnClickedApplyButton)
	ON_EN_CHANGE(IDC_LISTENPORT_SETTING, &CServerManagerDlg::OnEnChangeListenportSetting)
	ON_EN_CHANGE(IDC_MAX_CONNECTION_SETTING2, &CServerManagerDlg::OnEnChangeMaxConnectionSetting2)
END_MESSAGE_MAP()


// CServerManagerDlg 消息处理程序


void CServerManagerDlg::OnBnClickedApplyButton()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);  //更新修改的数据
	m_ConfigFile.SetIntToConfigFile(_T("Settings"), _T("ListenPort"), m_SetListenPort);
	//向ini文件中写入值
	m_ConfigFile.SetIntToConfigFile(_T("Settings"), _T("MaxConnections"), m_SetMaxConnection);
	SendMessage(WM_CLOSE);   //发送消息关闭本地窗口

}

//子窗口的初始化
BOOL CServerManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	int ListenPort = 0;
	int MaxConnections = 0;

	//m_ApplyButton.ShowWindow(SW_HIDE);
	m_ApplyButton.EnableWindow(FALSE);

	m_ConfigFile.GetIntFromConfigFile(_T("Settings"), _T("ListenPort"), (int*)&ListenPort);
	m_ConfigFile.GetIntFromConfigFile(_T("Settings"), _T("MaxConnections"), (int*)&MaxConnections);
	m_SetListenPort = ListenPort;
	m_SetMaxConnection= MaxConnections;

	//刷新窗口
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CServerManagerDlg::OnEnChangeListenportSetting()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	m_ApplyButton.ShowWindow(SW_NORMAL);
	m_ApplyButton.EnableWindow(TRUE);
}


void CServerManagerDlg::OnEnChangeMaxConnectionSetting2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	m_ApplyButton.ShowWindow(SW_NORMAL);
	m_ApplyButton.EnableWindow(TRUE);
}
