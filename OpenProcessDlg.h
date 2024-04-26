#pragma once
#include "afxdialogex.h"
#include"IocpServer.h"

// COpenProcessDlg 对话框

class COpenProcessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(COpenProcessDlg)

public:
	COpenProcessDlg::COpenProcessDlg(CWnd* pParent, CIocpServer*
		IocpServer, CONTEXT_OBJECT* ContextObject);
		
	virtual ~COpenProcessDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OPEN_PROCESS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_Description_Static;

	CIocpServer* m_IocpServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON   	m_IconHwnd;

	virtual BOOL OnInitDialog();
	CButton m_OK_Button;
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEditControl();
};
