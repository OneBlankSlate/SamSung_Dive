// VMMapDlg.cpp: 实现文件
//

#include "pch.h"
#include "SamSung Dive.h"
#include "afxdialogex.h"
#include "VMMapDlg.h"
#include<tchar.h>
#include"SamSung DiveDlg.h"
#include"Common.h"



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
	//DDX_Control(pDX, IDC_DETAILED_INFO_LIST, m_InfoList);
	DDX_Control(pDX, IDC_DETAILED_INFO_LIST, m_InfoList);
}


BEGIN_MESSAGE_MAP(CVMMapDlg, CDialogEx)
	
	
	ON_BN_CLICKED(IDC_QUERY_BUTTON, &CVMMapDlg::OnBnClickedQueryButton)
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
	
	int BufferLength = sizeof(BYTE) + 9*sizeof(size_t);
	LPBYTE BufferData = new BYTE[BufferLength];
	BufferData[0] = CLIENT_VIRTUAL_MEMORY_MAPPING_REQUIRE;
	m_IocpServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
	

	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CVMMapDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

}


void CVMMapDlg::OnBnClickedQueryButton()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CVMMapDlg::ShowSystemInfo()
{


	BYTE* BufferData = (BYTE*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));

	DWORD	Offset = 0;
	size_t v1;
	m_InfoList.DeleteAllItems();
	//遍历发送来的每一个字符别忘了他的数据结构啊 Id+进程名+0+完整名+0
	
	for (int i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		
		memcpy(&v1, BufferData + Offset, sizeof(size_t));
		
		CString v5;
		v5.Format("%#x", v1);
		m_edit1.SetWindowTextA(v5);
		

		//跳过这个数据结构进入下一个循环
		Offset += sizeof(size_t);

	}

}
