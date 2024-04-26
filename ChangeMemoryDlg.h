#pragma once
#include "afxdialogex.h"
#include"IocpServer.h"
#include"ProcessManagerDlg.h"

// CChangeMemoryDlg 对话框

class CChangeMemoryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChangeMemoryDlg)

public:
	CChangeMemoryDlg::CChangeMemoryDlg(CWnd* pParent, CIocpServer* IocpServer, CONTEXT_OBJECT* ContextObject);
	//CChangeMemoryDlg::CChangeMemoryDlg();
	virtual ~CChangeMemoryDlg();
	CIocpServer* m_IocpServer;
	PCONTEXT_OBJECT m_ContextObject;
	HANDLE m_ProcessIdentity;
	int m_TargetValue;
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_QUERY_MEMORY_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void ShowAddressInfoList();
	afx_msg void OnEnChangeProcessIdentity();
	afx_msg void OnBnClickedOkButton();
	afx_msg void OnBnClickedScanAgainButton();
	
private:
	CEdit m_edit1;
	CEdit m_edit2;
	CListCtrl m_AddressInfoList;

public:
	afx_msg void OnNMDblclkTargetAddressList(NMHDR* pNMHDR, LRESULT* pResult);
};

extern size_t __TargetAddress;