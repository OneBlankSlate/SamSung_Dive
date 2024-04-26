#pragma once
#include "IocpServer.h"

// CProcessManager �Ի���

class CProcessManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProcessManagerDlg)

public:
	CProcessManagerDlg(CWnd* pParent = NULL, CIocpServer* IocpServer = NULL, PCONTEXT_OBJECT ContextObject = NULL);  
	virtual ~CProcessManagerDlg();

	CIocpServer* m_IocpServer;
	PCONTEXT_OBJECT m_ContextObject;

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESS_MANAGER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	HICON   	m_IconHwnd;   //�û�����  �ں˶���   HWND  HANDLE  ���Ƕ��� 
	CListCtrl m_ClientInfo_List;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnNMCustomdrawProcessInofoList(NMHDR* pNMHDR, LRESULT* pResult);
//	afx_msg void OnNMRClickProcessInofoList(NMHDR* pNMHDR, LRESULT* pResult);
	void GetProcessInfoList();
	void ShowProcessInfoList(void);
	void OnReceiveComplete(void);
	CListCtrl m_ProcessInfoList;
	static DWORD WINAPI JudgeThreadProcedure(LPVOID ParameterData);
	
	afx_msg void OnRefreshProcess();
	afx_msg void OnKillProcess();
	afx_msg void OnSuspendProcess();
	afx_msg void OnResumeProcess();
	afx_msg void OnProcessModules();
	afx_msg void OnProcessThreads();
	afx_msg void OnProcessMemory();
	afx_msg void OnProcessHandles();
	afx_msg void OnRclickProcessInfoList(NMHDR* pNMHDR, LRESULT* pResult);

	void InitSolidMenu();
	VOID SendingSelectedCommand(PBYTE BufferData, ULONG BufferLength);
	
	
	afx_msg void OnCreateProcess();
	LRESULT OnOpenChangeMemoryDialog(WPARAM processID, LPARAM contextObject);

};
extern HANDLE __ProcessIdentity;