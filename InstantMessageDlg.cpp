// InstantMessageDlg.cpp: 实现文件
//

#include "pch.h"
#include "SamSung Dive.h"
#include "afxdialogex.h"
#include "InstantMessageDlg.h"
#include"Common.h"

// CInstantMessageDlg 对话框

IMPLEMENT_DYNAMIC(CInstantMessageDlg, CDialogEx)

CInstantMessageDlg::CInstantMessageDlg(CWnd* pParent , CIocpServer* IocpServer , PCONTEXT_OBJECT ContextObject )
	: CDialogEx(IDD_INSTANT_MESSAGE_DIALOG, pParent)
{
	m_IocpServer = IocpServer;
	m_ContextObject = ContextObject;
}

CInstantMessageDlg::~CInstantMessageDlg()
{
}

void CInstantMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INSTANT_MESSAGE_EDIT, m_InstantMessageEdit);
}


BEGIN_MESSAGE_MAP(CInstantMessageDlg, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


BOOL CInstantMessageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	BYTE IsToken = CLIENT_GO_ON;
	m_IocpServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

// CInstantMessageDlg 消息处理程序


void CInstantMessageDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);

	CDialogEx::OnClose();
}
BOOL CInstantMessageDlg::PreTranslateMessage(MSG* pMsg)  //键盘捕捉函数
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		//点击Esc键退出窗口
		if (pMsg->wParam == VK_ESCAPE)
		{
			PostMessage(WM_CLOSE);
			return true;
		}

		//判断是否在m_CEdit_Dialog_Remote_Message_Main控件上点击了回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_InstantMessageEdit.m_hWnd)
		{
			OnSending();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
void CInstantMessageDlg::OnSending()
{
	// TODO: 在此添加控件通知处理程序代码

	//获得m_CEdit_Dialog_Remote_Message_Main控件上的数据长度
	int BufferLength = m_InstantMessageEdit.GetWindowTextLength();
	if (!BufferLength)
	{
		return;
	}
	CString v1;
	//获得m_CEdit_Dialog_Remote_Message_Main控件上的数据内容并放到v1中(若是CString类型的控件变量，在从控件中得数据时上方要调用update）
	m_InstantMessageEdit.GetWindowText(v1);

	//IO通信套接字只支持char型数据
	char* BufferData = (char*)VirtualAlloc(NULL, BufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memset(BufferData, 0, sizeof(char) * BufferLength);
	sprintf(BufferData, "%s", v1.GetBuffer(0));

	//清空m_CEdit_Dialog_Remote_Message_Main控件上的数据
	m_InstantMessageEdit.SetWindowText(NULL);

	//调用通信引擎类中的发送数据函数
	m_IocpServer->OnPrepareSending(m_ContextObject,
		(LPBYTE)BufferData, strlen(BufferData));   //将自己内存中的数据发送

	if (BufferData != NULL)
	{
		VirtualFree(BufferData, BufferLength, MEM_RELEASE);
		BufferData = NULL;
	}
}