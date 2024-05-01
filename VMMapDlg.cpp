// VMMapDlg.cpp: 实现文件
//

#include "pch.h"
#include "SamSung Dive.h"
#include "afxdialogex.h"
#include "VMMapDlg.h"
#include<tchar.h>
#include"SamSung DiveDlg.h"
#include"Common.h"
#include"ProcessManagerDlg.h"

//静态数据成员一定要在类内定义，类外初始化
HANDLE CVMMapDlg::m_ProcessIdentity = INVALID_HANDLE_VALUE;
CString CVMMapDlg::m_ProcessPath = NULL;

COLUMN_DATA _Column_Data_Online[] =
{
	{ "ID",			50 },
	{ "地址",	70 },
	{ "大小",		70 },
	{ "状态",			70 },
	{ "类型",	70 },
	{ "初始保护",		70 },
	{ "访问保护",			70 }

};

// CVMMapDlg 对话框

IMPLEMENT_DYNAMIC(CVMMapDlg, CDialogEx)


CVMMapDlg::CVMMapDlg(CWnd* pParent, CIocpServer* IocpServer, CONTEXT_OBJECT* ContextObject):CDialogEx(IDD_VIRTUAL_MEMORY_MAPPING_DIALOG, pParent)
{

	m_IocpServer = IocpServer;
	m_ContextObject = ContextObject;
	m_ProcessIdentity = __ProcessIdentity;
}

CVMMapDlg::~CVMMapDlg()
{
}

void CVMMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MEMORY_PAGE_SIZE_EDIT, m_edit1);
	DDX_Control(pDX, IDC_MIN_ADDRESS_EDIT, m_edit2);
	DDX_Control(pDX, IDC_MAX_ADDRESS_EDIT, m_edit3);
	DDX_Control(pDX, IDC_PHYSICAL_TOTAL_MEMORY_EDIT, m_edit4);
	DDX_Control(pDX, IDC_TOTAL_PAGE_FILE_EDIT, m_edit5);
	DDX_Control(pDX, IDC_TOTAL_VIRTUAL_MEMORY_EDIT, m_edit6);
	DDX_Control(pDX, IDC_USABLE_PHYSICAL_MEMORY_EDIT, m_edit7);
	DDX_Control(pDX, IDC_USABLE_PAGE_FILE_EDIT, m_edit8);
	DDX_Control(pDX, IDC_USABLE_VIRTUAL_MEMORY_EDIT, m_edit9);
	DDX_Control(pDX, IDC_COMMIT_CHECK, m_Commit_Check);
	DDX_Control(pDX, IDC_RESERVE_CHECK, m_Reserve_Check);
	DDX_Control(pDX, IDC_FREE_CHECK, m_Free_Check);
	DDX_Control(pDX, IDC_MEMORY_INFO_LIST, m_MemoryInfoList);
	DDX_Control(pDX, IDC_PROCESS_IDENTITY_EDIT, m_ProcessIdentity_edit);
	DDX_Control(pDX, IDC_PROCESS_FULL_PATH_EDIT, m_ProcessFullPath_edit);
}


BEGIN_MESSAGE_MAP(CVMMapDlg, CDialogEx)
	
	ON_WM_TIMER()
	
	ON_BN_CLICKED(IDC_UPDATE_BUTTON, &CVMMapDlg::OnBnClickedUpdateButton)

END_MESSAGE_MAP()


// CVMMapDlg 消息处理程序


void CVMMapDlg::OnEnChangeEdit6()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


BOOL CVMMapDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 
	//初始化进程ID与进程完整路径
	CString Cstr;
	Cstr.Format("%d", m_ProcessIdentity);
	m_ProcessIdentity_edit.SetWindowTextA(Cstr);
	m_ProcessFullPath_edit.SetWindowTextA(m_ProcessPath);

	//初始化ListControl的第一行属性
	for (int i = 0; i <
		sizeof(_Column_Data_Online) / sizeof(COLUMN_DATA); i++)
	{
		m_MemoryInfoList.InsertColumn(i, _Column_Data_Online[i].TitleData,
			LVCFMT_CENTER, _Column_Data_Online[i].TitleWidth);
	}
	m_MemoryInfoList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE);

	this->m_Commit_Check.SetCheck(BST_CHECKED);
	this->m_Reserve_Check.SetCheck(BST_CHECKED);
	this->m_Free_Check.SetCheck(BST_UNCHECKED);


	this->SetTimer(1, 1000, NULL);
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



//void CVMMapDlg::OnBnClickedQueryButton()
//{
	// TODO: 在此添加控件通知处理程序代码

//}

void CVMMapDlg::ShowClientInfo()
{
	DWORD	Offset = 0;
	PBYTE BufferData = (PBYTE)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	SYSTEM_INFO SystemInfo;
	MEMORYSTATUS MemoryStatus;
	memcpy(&SystemInfo, BufferData + Offset, sizeof(SYSTEM_INFO));
	Offset += sizeof(SYSTEM_INFO);
	memcpy(&MemoryStatus, BufferData + Offset, sizeof(MEMORYSTATUS));
	Offset += sizeof(MEMORYSTATUS);

	CString v1;

	//内存页大小
	v1.Format(_T("%p"), SystemInfo.dwPageSize);
	m_edit1.SetWindowTextA(v1);
	
	//最小地址
	v1.Format(_T("%p"), SystemInfo.lpMinimumApplicationAddress);
	m_edit2.SetWindowTextA(v1);

	//最大地址
	v1.Format(_T("%p"), SystemInfo.lpMaximumApplicationAddress);
	m_edit3.SetWindowTextA(v1);

	//总物理内存
	v1.Format(_T("%p"), MemoryStatus.dwTotalPhys);
	m_edit4.SetWindowTextA(v1);

	//总页文件
	v1.Format(_T("%p"), MemoryStatus.dwTotalPageFile);
	m_edit5.SetWindowTextA(v1);
	
	//总虚拟内存
	v1.Format(_T("%p"), MemoryStatus.dwTotalVirtual);
	m_edit6.SetWindowTextA(v1);

	//可用物理内存
	v1.Format(_T("%p"), MemoryStatus.dwAvailPhys);
	m_edit7.SetWindowTextA(v1);

	//可用页文件
	v1.Format(_T("%p"), MemoryStatus.dwAvailPageFile);
	m_edit8.SetWindowTextA(v1);
	
	//可用虚拟内存
	v1.Format(_T("%p"), MemoryStatus.dwAvailVirtual);
	m_edit9.SetWindowTextA(v1);


	if (m_MemoryBasicInfoList.size() == 0) {
		PBYTE bufferData = BufferData;
		DWORD offset = Offset;
		for (int i = 0; offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++, offset += sizeof(MEMORY_BASIC_INFORMATION))
		{
			//int test = m_CommitCheckBox.GetCheck();
			MEMORY_BASIC_INFORMATION* mbi = (MEMORY_BASIC_INFORMATION*)(bufferData + offset);
			if ((mbi->State != MEM_COMMIT) && (mbi->State != MEM_RESERVE) && (mbi->State != MEM_FREE))
				continue;
			m_MemoryBasicInfoList.push_back(*mbi);
		}
		CString v1;
		UINT index = 0;
		list<MEMORY_BASIC_INFORMATION>::iterator Travel;
		for (Travel = this->m_MemoryBasicInfoList.begin();
			Travel != this->m_MemoryBasicInfoList.end(); Travel++)
		{
			//if ((Travel->State == MEM_COMMIT && this->m_Commit_Check.GetCheck() == BST_UNCHECKED) || \
			//	(Travel->State == MEM_RESERVE && this->m_Reserve_Check.GetCheck() == BST_UNCHECKED) || \
			//	(Travel->State == MEM_FREE && this->m_Free_Check.GetCheck() == BST_UNCHECKED) || \
			//	(Travel->State == 0))
			//	continue;

			v1.Format(_T("%d"), index + 1);
			this->m_MemoryInfoList.InsertItem(index, v1);

			// [1]起始地址
			v1.Format(_T("%p"), Travel->BaseAddress);
			this->m_MemoryInfoList.SetItemText(index, 1, v1);

			// [2]大小
			v1.Format(_T("%p"), Travel->RegionSize);
			this->m_MemoryInfoList.SetItemText(index, 2, v1);

			// [3]状态
			switch (Travel->State)
			{
			case MEM_COMMIT:	v1 = _T("提交");	break;
			case MEM_RESERVE:	v1 = _T("保留");	break;
			case MEM_FREE:		v1 = _T("空闲");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 3, v1);

			// [4]类型
			switch (Travel->Type)
			{
			case MEM_IMAGE:		v1 = _T("映像");	break;
			case MEM_PRIVATE:	v1 = _T("私有");	break;
			case MEM_MAPPED:	v1 = _T("映射");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 4, v1);

			// [5]初始保护
			switch (Travel->AllocationProtect)
			{
			case PAGE_READONLY:				v1 = _T("-R--");	break;
			case PAGE_READWRITE:			v1 = _T("-RW-");	break;
			case PAGE_WRITECOPY:			v1 = _T("-RWC");	break;
			case PAGE_EXECUTE:				v1 = _T("E---");	break;
			case PAGE_EXECUTE_READ:			v1 = _T("ER--");	break;
			case PAGE_EXECUTE_READWRITE:	v1 = _T("ERW-");	break;
			case PAGE_EXECUTE_WRITECOPY:	v1 = _T("ERWC");	break;
			case PAGE_NOACCESS:				v1 = _T("----");	break;
			default:						v1 = _T("----");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 5, v1);


			// [6]访问保护
			switch (Travel->Protect)
			{
			case PAGE_READONLY:				v1 = _T("-R--");	break;
			case PAGE_READWRITE:			v1 = _T("-RW-");	break;
			case PAGE_WRITECOPY:			v1 = _T("-RWC");	break;
			case PAGE_EXECUTE:				v1 = _T("E---");	break;
			case PAGE_EXECUTE_READ:			v1 = _T("ER--");	break;
			case PAGE_EXECUTE_READWRITE:	v1 = _T("ERW-");	break;
			case PAGE_EXECUTE_WRITECOPY:	v1 = _T("ERWC");	break;
			case PAGE_NOACCESS:				v1 = _T("----");	break;
			default:						v1 = _T("----");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 6, v1);

			index++;
		}
	}
	else {
		CString v1;
		UINT index = 0;
		list<MEMORY_BASIC_INFORMATION>::iterator Travel;
		for (Travel = this->m_MemoryBasicInfoList.begin();
			Travel != this->m_MemoryBasicInfoList.end(); Travel++)
		{
			if ((Travel->State == MEM_COMMIT && this->m_Commit_Check.GetCheck() == BST_UNCHECKED) || \
				(Travel->State == MEM_RESERVE && this->m_Reserve_Check.GetCheck() == BST_UNCHECKED) || \
				(Travel->State == MEM_FREE && this->m_Free_Check.GetCheck() == BST_UNCHECKED) || \
				(Travel->State == 0))
				continue;

			v1.Format(_T("%d"), index + 1);
			this->m_MemoryInfoList.InsertItem(index, v1);

			// [1]起始地址
			v1.Format(_T("%p"), Travel->BaseAddress);
			this->m_MemoryInfoList.SetItemText(index, 1, v1);

			// [2]大小
			v1.Format(_T("%p"), Travel->RegionSize);
			this->m_MemoryInfoList.SetItemText(index, 2, v1);

			// [3]状态
			switch (Travel->State)
			{
			case MEM_COMMIT:	v1 = _T("提交");	break;
			case MEM_RESERVE:	v1 = _T("保留");	break;
			case MEM_FREE:		v1 = _T("空闲");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 3, v1);

			// [4]类型
			switch (Travel->Type)
			{
			case MEM_IMAGE:		v1 = _T("映像");	break;
			case MEM_PRIVATE:	v1 = _T("私有");	break;
			case MEM_MAPPED:	v1 = _T("映射");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 4, v1);

			// [5]初始保护
			switch (Travel->AllocationProtect)
			{
			case PAGE_READONLY:				v1 = _T("-R--");	break;
			case PAGE_READWRITE:			v1 = _T("-RW-");	break;
			case PAGE_WRITECOPY:			v1 = _T("-RWC");	break;
			case PAGE_EXECUTE:				v1 = _T("E---");	break;
			case PAGE_EXECUTE_READ:			v1 = _T("ER--");	break;
			case PAGE_EXECUTE_READWRITE:	v1 = _T("ERW-");	break;
			case PAGE_EXECUTE_WRITECOPY:	v1 = _T("ERWC");	break;
			case PAGE_NOACCESS:				v1 = _T("----");	break;
			default:						v1 = _T("----");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 5, v1);


			// [6]访问保护
			switch (Travel->Protect)
			{
			case PAGE_READONLY:				v1 = _T("-R--");	break;
			case PAGE_READWRITE:			v1 = _T("-RW-");	break;
			case PAGE_WRITECOPY:			v1 = _T("-RWC");	break;
			case PAGE_EXECUTE:				v1 = _T("E---");	break;
			case PAGE_EXECUTE_READ:			v1 = _T("ER--");	break;
			case PAGE_EXECUTE_READWRITE:	v1 = _T("ERW-");	break;
			case PAGE_EXECUTE_WRITECOPY:	v1 = _T("ERWC");	break;
			case PAGE_NOACCESS:				v1 = _T("----");	break;
			default:						v1 = _T("----");	break;
			}
			this->m_MemoryInfoList.SetItemText(index, 6, v1);

			index++;
		}
	}
	
}

void CVMMapDlg::UpdateSystemInfo()
{
	DWORD	Offset = 0;
	PBYTE BufferData = (PBYTE)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	SYSTEM_INFO SystemInfo;
	MEMORYSTATUS MemoryStatus;
	memcpy(&SystemInfo, BufferData + Offset, sizeof(SYSTEM_INFO));
	Offset += sizeof(SYSTEM_INFO);
	memcpy(&MemoryStatus, BufferData + Offset, sizeof(MEMORYSTATUS));
	Offset += sizeof(MEMORYSTATUS);

	CString v1;

	//内存页大小
	v1.Format(_T("%p"), SystemInfo.dwPageSize);
	m_edit1.SetWindowTextA(v1);

	//最小地址
	v1.Format(_T("%p"), SystemInfo.lpMinimumApplicationAddress);
	m_edit2.SetWindowTextA(v1);

	//最大地址
	v1.Format(_T("%p"), SystemInfo.lpMaximumApplicationAddress);
	m_edit3.SetWindowTextA(v1);

	//总物理内存
	v1.Format(_T("%p"), MemoryStatus.dwTotalPhys);
	m_edit4.SetWindowTextA(v1);

	//总页文件
	v1.Format(_T("%p"), MemoryStatus.dwTotalPageFile);
	m_edit5.SetWindowTextA(v1);

	//总虚拟内存
	v1.Format(_T("%p"), MemoryStatus.dwTotalVirtual);
	m_edit6.SetWindowTextA(v1);

	//可用物理内存
	v1.Format(_T("%p"), MemoryStatus.dwAvailPhys);
	m_edit7.SetWindowTextA(v1);

	//可用页文件
	v1.Format(_T("%p"), MemoryStatus.dwAvailPageFile);
	m_edit8.SetWindowTextA(v1);

	//可用虚拟内存
	v1.Format(_T("%p"), MemoryStatus.dwAvailVirtual);
	m_edit9.SetWindowTextA(v1);
}



void CVMMapDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	BYTE Token = CLIENT_UPDATE_VMMAP_SYSTEM_INFO_REQUIRE;
	HANDLE ProcessIdentity= m_ProcessIdentity;
	int BufferLength =sizeof(BYTE)+sizeof(HANDLE);
	LPBYTE BufferData = new BYTE[BufferLength];
	BufferData[0] = Token;
	memcpy(BufferData + sizeof(BYTE), &ProcessIdentity, sizeof(HANDLE));
	this->m_IocpServer->OnPrepareSending(m_ContextObject,BufferData, BufferLength);

	CDialogEx::OnTimer(nIDEvent);
}



void CVMMapDlg::OnBnClickedUpdateButton()
{
	// TODO: 在此添加控件通知处理程序代码
	//在已有列表中筛选
	m_MemoryInfoList.DeleteAllItems();
	CString v1;
	UINT index = 0;
	list<MEMORY_BASIC_INFORMATION>::iterator Travel;
	for (Travel = this->m_MemoryBasicInfoList.begin();
		Travel != this->m_MemoryBasicInfoList.end(); Travel++)
	{
		if ((Travel->State == MEM_COMMIT && this->m_Commit_Check.GetCheck() == BST_UNCHECKED) || \
			(Travel->State == MEM_RESERVE && this->m_Reserve_Check.GetCheck() == BST_UNCHECKED) || \
			(Travel->State == MEM_FREE && this->m_Free_Check.GetCheck() == BST_UNCHECKED) || \
			(Travel->State == 0))
			continue;

		v1.Format(_T("%d"), index + 1);
		this->m_MemoryInfoList.InsertItem(index, v1);

		// [1]起始地址
		v1.Format(_T("%p"), Travel->BaseAddress);
		this->m_MemoryInfoList.SetItemText(index, 1, v1);

		// [2]大小
		v1.Format(_T("%p"), Travel->RegionSize);
		this->m_MemoryInfoList.SetItemText(index, 2, v1);

		// [3]状态
		switch (Travel->State)
		{
		case MEM_COMMIT:	v1 = _T("提交");	break;
		case MEM_RESERVE:	v1 = _T("保留");	break;
		case MEM_FREE:		v1 = _T("空闲");	break;
		}
		this->m_MemoryInfoList.SetItemText(index, 3, v1);

		// [4]类型
		switch (Travel->Type)
		{
		case MEM_IMAGE:		v1 = _T("映像");	break;
		case MEM_PRIVATE:	v1 = _T("私有");	break;
		case MEM_MAPPED:	v1 = _T("映射");	break;
		}
		this->m_MemoryInfoList.SetItemText(index, 4, v1);

		// [5]初始保护
		switch (Travel->AllocationProtect)
		{
		case PAGE_READONLY:				v1 = _T("-R--");	break;
		case PAGE_READWRITE:			v1 = _T("-RW-");	break;
		case PAGE_WRITECOPY:			v1 = _T("-RWC");	break;
		case PAGE_EXECUTE:				v1 = _T("E---");	break;
		case PAGE_EXECUTE_READ:			v1 = _T("ER--");	break;
		case PAGE_EXECUTE_READWRITE:	v1 = _T("ERW-");	break;
		case PAGE_EXECUTE_WRITECOPY:	v1 = _T("ERWC");	break;
		case PAGE_NOACCESS:				v1 = _T("----");	break;
		default:						v1 = _T("----");	break;
		}
		this->m_MemoryInfoList.SetItemText(index, 5, v1);


		// [6]访问保护
		switch (Travel->Protect)
		{
		case PAGE_READONLY:				v1 = _T("-R--");	break;
		case PAGE_READWRITE:			v1 = _T("-RW-");	break;
		case PAGE_WRITECOPY:			v1 = _T("-RWC");	break;
		case PAGE_EXECUTE:				v1 = _T("E---");	break;
		case PAGE_EXECUTE_READ:			v1 = _T("ER--");	break;
		case PAGE_EXECUTE_READWRITE:	v1 = _T("ERW-");	break;
		case PAGE_EXECUTE_WRITECOPY:	v1 = _T("ERWC");	break;
		case PAGE_NOACCESS:				v1 = _T("----");	break;
		default:						v1 = _T("----");	break;
		}
		this->m_MemoryInfoList.SetItemText(index, 6, v1);

		index++;


	}
}
