#pragma once
#include "afxdialogex.h"
#include"IocpServer.h"
#include<tchar.h>
#include"_VMMap.h"




// CVMMapDlg 对话框

class CVMMapDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVMMapDlg)

public:
	CVMMapDlg::CVMMapDlg(CWnd* pParent, CIocpServer* IocpServer, CONTEXT_OBJECT* ContextObject);
	virtual ~CVMMapDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIRTUAL_MEMORY_MAPPING_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CIocpServer* m_IocpServer;
	PCONTEXT_OBJECT m_ContextObject;
	
public:
	afx_msg void OnEnChangeEdit6();
	CEdit m_edit1;
	CEdit m_edit2;
	CEdit m_edit3;
	CEdit m_edit4;
	CEdit m_edit5;
	CEdit m_edit6;
	CEdit m_edit7;
	CEdit m_edit8;
	CEdit m_edit9;
	list<MEMORY_BASIC_INFORMATION> m_MemoryBasicInfoList;
	virtual BOOL OnInitDialog();
//	CListCtrl m_InfoList;
//	afx_msg void OnBnClickedButton1();
	
//	afx_msg void OnBnClickedQueryButton();
	void ShowClientInfo();
	void UpdateSystemInfo();
	CButton m_Commit_Check;
	CButton m_Reserve_Check;
	CButton m_Free_Check;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedUpdateButton();
	CListCtrl m_MemoryInfoList;
	static HANDLE m_ProcessIdentity;
	static CString m_ProcessPath;
	CEdit m_ProcessIdentity_edit;
	CEdit m_ProcessFullPath_edit;
};
