// EditValueDlg.cpp: 实现文件
//

#include "pch.h"
#include "SamSung Dive.h"
#include "afxdialogex.h"
#include "EditValueDlg.h"



// CEditValueDlg 对话框

IMPLEMENT_DYNAMIC(CEditValueDlg, CDialogEx)

CEditValueDlg::CEditValueDlg(CWnd* pParent, CString* Address, CIocpServer*
	IocpServer, CONTEXT_OBJECT* ContextObject)
	: CDialogEx(IDD_CHANGE_VALUE_DIALOG, pParent)
{
	m_Parent = (CChangeMemoryDlg*)pParent;
	m_Adddress = Address;
	m_IocpServer = IocpServer;
	m_ContextObject = ContextObject;
}

CEditValueDlg::~CEditValueDlg()
{
}

void CEditValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TARGET_ADDRESS_EDIT, m_TargetAddressEdit);
	DDX_Control(pDX, IDC_NEW_VALUE_EDIT_CONTROL, m_NewValueEdit);
}


BEGIN_MESSAGE_MAP(CEditValueDlg, CDialogEx)
	ON_EN_CHANGE(IDC_ADDRESS_EDIT_CONTROL, &CEditValueDlg::OnEnChangeAddressEditControl)
	ON_BN_CLICKED(IDC_YES_BUTTON, &CEditValueDlg::OnBnClickedYesButton)
END_MESSAGE_MAP()


// CEditValueDlg 消息处理程序


void CEditValueDlg::OnEnChangeAddressEditControl()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


BOOL CEditValueDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_TargetAddressEdit.SetWindowTextA(*m_Adddress);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CEditValueDlg::OnBnClickedYesButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString NewValueStr;
	m_NewValueEdit.GetWindowTextA(NewValueStr);
	int NewValue = _ttoi(NewValueStr);
	int BufferLength = sizeof(BYTE) + sizeof(int) + sizeof(size_t);
	BYTE* BufferData = new BYTE[BufferLength];
	BufferData[0] = CLIENT_EDIT_VALUE_REQUIRE;
	memcpy(BufferData + sizeof(BYTE), &__TargetAddress, sizeof(size_t));
	memcpy(BufferData + sizeof(BYTE) + sizeof(size_t), &NewValue, sizeof(int));
	
	m_IocpServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
}
