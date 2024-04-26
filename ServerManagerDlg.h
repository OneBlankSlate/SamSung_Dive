#pragma once
#include "afxdialogex.h"
#include"ConfigFile.h"

// CServerManagerDlg 对话框

class CServerManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CServerManagerDlg)

public:
	CServerManagerDlg(CWnd* pParent = NULL);	 //重构构造函数

	virtual ~CServerManagerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_MANAGER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedApplyButton();
	virtual BOOL OnInitDialog();
	CConfigFile m_ConfigFile;
	int m_SetListenPort;
	int m_SetMaxConnection;
	afx_msg void OnEnChangeListenportSetting();
	afx_msg void OnEnChangeMaxConnectionSetting2();
	CButton m_ApplyButton;

};
