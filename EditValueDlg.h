#pragma once
#include "afxdialogex.h"
#include"IocpServer.h"
#include"ChangeMemoryDlg.h"
#include "atlstr.h"
#include"Common.h"
// CEditValueDlg 对话框

class CEditValueDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditValueDlg)

public:
	CEditValueDlg(CWnd* pParent = nullptr);   // 标准构造函数

	
	CEditValueDlg(CWnd* pParent, CString* Address, CIocpServer*
		IocpServer, CONTEXT_OBJECT* ContextObject);
		

	virtual ~CEditValueDlg();

	CIocpServer* m_IocpServer;
	PCONTEXT_OBJECT m_ContextObject;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHANGE_VALUE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeAddressEditControl();
	virtual BOOL OnInitDialog();
	CString* m_Adddress;
	CChangeMemoryDlg* m_Parent = nullptr;
	
	CEdit m_TargetAddressEdit;
	afx_msg void OnBnClickedYesButton();
	CEdit m_NewValueEdit;
};
