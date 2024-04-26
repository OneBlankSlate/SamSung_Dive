#include "pch.h"
#include "Samsung Dive.h"
#include "ProcessManagerDlg.h"
#include "afxdialogex.h"
#include "Common.h"
#include"OpenProcessDlg.h"
#include"ChangeMemoryDlg.h"
HANDLE __ProcessIdentity;
// CProcessManager �Ի���
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


// CProcessManager ��Ϣ�������


BOOL CProcessManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_IconHwnd, FALSE);
	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("\\\\%s - Զ�̽��̹���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);



	//���öԻ���������
	LOGFONT  Logfont;
	CFont* v2 = m_ProcessInfoList.GetFont();
	v2->GetLogFont(&Logfont);
	//��������
	Logfont.lfHeight = Logfont.lfHeight * 1.3;   //��������޸�����ĸ߱���
	Logfont.lfWidth = Logfont.lfWidth * 1.3;     //��������޸�����Ŀ����
	CFont  v3;
	v3.CreateFontIndirect(&Logfont);
	m_ProcessInfoList.SetFont(&v3);
	m_ProcessInfoList.SetFont(&v3);
	v3.Detach();




	//���ض˴��ص�����
	char* BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0));
	m_ProcessInfoList.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

	switch (BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_REPLY:
	{

		m_ProcessInfoList.InsertColumn(0, _T("PID"), LVCFMT_LEFT, 80);
		m_ProcessInfoList.InsertColumn(1, _T("���̾���"), LVCFMT_LEFT, 180);
		m_ProcessInfoList.InsertColumn(2, _T("����·��"), LVCFMT_LEFT, 250);
		m_ProcessInfoList.InsertColumn(3, _T("����λ��"), LVCFMT_LEFT, 120);
		m_ProcessInfoList.InsertColumn(4, _T("�������"), LVCFMT_LEFT, 120);
		ShowProcessInfoList();   //���ڵ�һ������������Ϣ��������Ž��̵��������԰�������ʾ���б���\0\0
		break;
	}
	default:
		break;
	}

	InitSolidMenu();
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CProcessManagerDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	SetForegroundWindow();   //���õ�ǰ����
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
			NewTextColor = RGB(0, 0, 0);          //��ɫ
			NewBackgroundColor = RGB(255, 0, 0);    //��ɫ       RGB(0,255,0)��ɫ
		}
		else if (_tcscmp(ProcessImageName.GetBuffer(0), _T("CalculatorApp.exe")) == 0)
		{
			NewTextColor = RGB(0, 0, 0);         //��ɫ
			NewBackgroundColor = RGB(0, 0, 255);   //��ɫ
		}

		else {
			NewTextColor = RGB(0, 0, 0);          //��ɫ
			NewBackgroundColor = RGB(255, 255, 255);    //��ɫ
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
	//������������ÿһ���ַ��������������ݽṹ�� Id+������+0+������+0
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		HANDLE* ProcessIdentity = (HANDLE*)(BufferData + Offset);              //����ID
		ProcessImageName = BufferData + Offset + sizeof(HANDLE);         //������
		ProcessFullPath = ProcessImageName + lstrlen(ProcessImageName) + 1;            //��������·��
		IsWow64Process = ProcessFullPath + lstrlen(ProcessFullPath) + 1;

		v1.Format(_T("%5u"), *ProcessIdentity);
		m_ProcessInfoList.InsertItem(i, v1);       //���õ������ݼ��뵽�б���
		m_ProcessInfoList.SetItemText(i, 1, ProcessImageName);
		m_ProcessInfoList.SetItemText(i, 2, ProcessFullPath);
		m_ProcessInfoList.SetItemText(i, 3, IsWow64Process);
		//ItemData Ϊ����ID
		m_ProcessInfoList.SetItemData(i, (DWORD_PTR)*ProcessIdentity);   //����Ϊ��������
		//����������ݽṹ������һ��ѭ��
		Offset += sizeof(HANDLE) + lstrlen(ProcessImageName) + lstrlen(ProcessFullPath) + lstrlen(IsWow64Process) + 3;

	}

	v1.Format(_T("������� / %d"), i);
	LVCOLUMN v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_ProcessInfoList.SetColumn(4, &v3); //���б�����ʾ�ж��ٸ�����
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

//�����ݰ��������ͻ���
VOID CProcessManagerDlg::SendingSelectedCommand(PBYTE BufferData, ULONG BufferLength)
{

	//��ListControl�ϵ���������ѡȡ��Context
	POSITION Position = m_ClientInfo_List.GetFirstSelectedItemPosition();
	//�ô���֧�ֶ���ѡ��
	while (Position)
	{
		int	Item = m_ClientInfo_List.GetNextSelectedItem(Position);
		//��ø��ŵ�����������õ�Context
		CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)m_ClientInfo_List.GetItemData(Item);   //������ʾ�ĺ����в���һ��Context��������


		//ͨ���ฺ��������
		m_IocpServer->OnPrepareSending(ContextObject, BufferData, BufferLength);

	}
}


void CProcessManagerDlg::OnRefreshProcess()
{

	// TODO: �ڴ���������������
	GetProcessInfoList();
}


void CProcessManagerDlg::OnKillProcess()
{
	// TODO: �ڴ���������������

	CListCtrl* ListCtrl = NULL;
	if (m_ProcessInfoList.IsWindowVisible())
		ListCtrl = &m_ProcessInfoList;
	else
		return;

	//[KILL][ID][ID][iD][ID]
	//���仺����

	//��������  ��·����
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (ListCtrl->GetSelectedCount() * sizeof(HANDLE)));//1.exe  4  ID   Handle
	//����������̵�����ͷ
	BufferData[0] = CLIENT_PROCESS_MANAGER_KILL_REQUIRE;
	//��ʾ������Ϣ
	TCHAR* Tips = _T("����: ��ֹ���̻ᵼ�²�ϣ�������Ľ����\n"
		"�������ݶ�ʧ��ϵͳ���ȶ����ڱ���ֹǰ��\n"
		"���̽�û�л��ᱣ����״̬�����ݡ�");
	CString v1;

	//ѡ���˼������
	if (ListCtrl->GetSelectedCount() > 1)
	{
		v1.Format(_T("%sȷʵ\n����ֹ��%d�������?"), Tips, ListCtrl->GetSelectedCount());
	}
	else
	{
		v1.Format(_T("%sȷʵ\n����ֹ���������?"), Tips);
	}
	//SDK 
	if (::MessageBox(m_hWnd, v1, _T("���̽�������"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;
	//MFC
	/*
	if (MessageBox(v1, _T("���̽�������", MB_YESNO | MB_ICONQUESTION))== IDNO)
	{
		return;
	}
	*/
	//Offset
	//[Flag][][][][][]
	DWORD	Offset = 1;
	POSITION Position = ListCtrl->GetFirstSelectedItemPosition();
	//�õ�Ҫ�����ĸ�����

	while (Position)
	{
		int	Item = ListCtrl->GetNextSelectedItem(Position);
		HANDLE ProcessIdentity = (HANDLE)ListCtrl->GetItemData(Item);
		memcpy(BufferData + Offset, &ProcessIdentity, sizeof(HANDLE));  
		Offset += sizeof(HANDLE);
	}
	//�������ݵ����ض��ڱ��ض��в���CLIENT_PROCESS_MANAGER_KILL_REQUIRE�������ͷ
	m_IocpServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);

	Sleep(100);

	//�Է���Ϣ�ķ�ʽ���ú���������һ���߳���ִ��Ŀ�꺯������ֹ��ѭ��
	PostMessage(WM_COMMAND, MAKEWPARAM(ID_REFRESH_PROCESS, BN_CLICKED));

}
void CProcessManagerDlg::InitSolidMenu()
{
	HMENU  MenuObject;    //����һ���˵�����    ������������ת�����ַ�������
	MenuObject = LoadMenu(NULL, MAKEINTRESOURCE(IDR_PROCESS_MANAGER_DIALOG_MAIN_MENU));        //���˵���������˵���Դ   ����ԴIDת�����ַ�������
	::SetMenu(this->GetSafeHwnd(), MenuObject);                                       //��������Դ�Ĳ˵��������õ���ǰ�����ھ����
	::DrawMenuBar(this->GetSafeHwnd());              //����������::�ĺ�������ΪSDK�ķ���
	
}


void CProcessManagerDlg::OnSuspendProcess()
{
	// TODO: �ڴ���������������
}

void CProcessManagerDlg::OnResumeProcess()
{
	// TODO: �ڴ���������������
}




void CProcessManagerDlg::OnProcessModules()
{
	// TODO: �ڴ���������������
}


void CProcessManagerDlg::OnProcessThreads()
{
	// TODO: �ڴ���������������
}


void CProcessManagerDlg::OnProcessMemory()
{


	// TODO: �ڴ���������������
	POSITION Position = this->m_ProcessInfoList.GetFirstSelectedItemPosition();
	int Item = this->m_ProcessInfoList.GetNextSelectedItem(Position);
	__ProcessIdentity = (HANDLE)(this->m_ProcessInfoList.GetItemData(Item));
	
		
	int BufferLength = sizeof(HANDLE) + sizeof(BYTE);
	LPBYTE BufferData = new BYTE[BufferLength];
	BufferData[0] = CLIENT_REMOTE_CHANGE_MEMORY_REQUIRE;
	memcpy(BufferData + sizeof(BYTE), &__ProcessIdentity, sizeof(HANDLE));

	m_IocpServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, BufferLength);

	//������������
	PostMessage(UM_OPEN_CHANGE_MEMORY_DIALOG, (WPARAM)__ProcessIdentity, (LPARAM)m_ContextObject);



}


void CProcessManagerDlg::OnProcessHandles()
{
	// TODO: �ڴ���������������
}





void CProcessManagerDlg::OnCreateProcess()
{
	// TODO: �ڴ���������������
	
	////��̬����  
	COpenProcessDlg* Dialog = new COpenProcessDlg(this, m_IocpServer, m_ContextObject);
	// ���ø�����Ϊ����
	Dialog->Create(IDD_OPEN_PROCESS_DIALOG, GetDesktopWindow());    //������������Dlg
	Dialog->ShowWindow(SW_SHOW);
	Sleep(100);
	PostMessage(WM_COMMAND, MAKEWPARAM(ID_REFRESH_PROCESS, BN_CLICKED));
}

LRESULT CProcessManagerDlg::OnOpenChangeMemoryDialog(WPARAM processID, LPARAM contextObject)
{

	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)contextObject;

	//�������Ի���
	CChangeMemoryDlg* Dialog = new CChangeMemoryDlg(this, m_IocpServer, ContextObject);
	//m_CreateProcessDlg = Dialog;
	// ���ø�����Ϊ׿��
	Dialog->Create(IDD_QUERY_MEMORY_DIALOG, GetDesktopWindow());    //������������Dlg
	Dialog->ShowWindow(SW_SHOW);


	m_ContextObject->ChangeMemoryDlg = Dialog;
	return 0;
}