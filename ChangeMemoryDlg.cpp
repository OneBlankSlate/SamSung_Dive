
#include "pch.h"
#include "SamSung Dive.h"
#include "afxdialogex.h"
#include "ChangeMemoryDlg.h"
#include"Common.h"
#include"EditValueDlg.h"
#include"ProcessHelper.h" 
#include"SamSung DiveDlg.h"
HANDLE GlobalProcessHandle = 0;
size_t __TargetAddress = 0;


/*****Client表头*****/
COLUMN_DATA __AddressInfoList[] =
{
	{ "Address",	    240 },
	{ "Value",			200 },
};
// CChangeMemoryDlg 对话框

IMPLEMENT_DYNAMIC(CChangeMemoryDlg, CDialogEx)



CChangeMemoryDlg::CChangeMemoryDlg(CWnd* pParent, CIocpServer*
	IocpServer, CONTEXT_OBJECT* ContextObject)
	: CDialogEx(IDD_PROCESS_MANAGER_DIALOG, pParent)
{

	m_IocpServer = IocpServer;
	m_ContextObject = ContextObject;
	m_ProcessIdentity = INVALID_HANDLE_VALUE;
	m_TargetValue = 0;



}

CChangeMemoryDlg::~CChangeMemoryDlg()
{
}

void CChangeMemoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESS_IDENTITY, m_edit1);
	DDX_Control(pDX, IDC_TARGET_VALUE, m_edit2);
	DDX_Control(pDX, IDC_TARGET_ADDRESS_LIST, m_AddressInfoList);
}


BEGIN_MESSAGE_MAP(CChangeMemoryDlg, CDialogEx)
	ON_EN_CHANGE(IDC_PROCESS_IDENTITY, &CChangeMemoryDlg::OnEnChangeProcessIdentity)
	ON_BN_CLICKED(IDC_OK_BUTTON, &CChangeMemoryDlg::OnBnClickedOkButton)
	ON_BN_CLICKED(IDC_SCAN_AGAIN_BUTTON, &CChangeMemoryDlg::OnBnClickedScanAgainButton)
	ON_NOTIFY(NM_DBLCLK, IDC_TARGET_ADDRESS_LIST, &CChangeMemoryDlg::OnNMDblclkTargetAddressList)

END_MESSAGE_MAP()


// CChangeMemoryDlg 消息处理程序


BOOL CChangeMemoryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	
	TCHAR v1[10]={0};
	//memcpy(v1, __ProcessIdentity, sizeof(HANDLE)); 

	_stprintf(v1, _T("%u"), (DWORD)__ProcessIdentity);

	/*char* v1 = 0;
	itoa((int)__ProcessIdentity, v1, 10);*/
	m_edit1.SetWindowText(v1);

	for (int i = 0; i < sizeof(__AddressInfoList) / sizeof(COLUMN_DATA); i++)
	{
		//设置ClientInfoList的标题栏
		m_AddressInfoList.InsertColumn(i, __AddressInfoList[i].TitleData,
			LVCFMT_CENTER, __AddressInfoList[i].TitleWidth);
		//LVCFMT_CENTER是文字居中
	}
	m_AddressInfoList.SetExtendedStyle(LVS_EX_FULLROWSELECT);


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CChangeMemoryDlg::OnEnChangeProcessIdentity()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void CChangeMemoryDlg::ShowAddressInfoList()
{
	
	BYTE* BufferData = (BYTE*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));

	DWORD	Offset = 0;
	size_t v1;
	m_AddressInfoList.DeleteAllItems();
	//遍历发送来的每一个字符别忘了他的数据结构啊 Id+进程名+0+完整名+0
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		size_t Address;
		memcpy(&Address, BufferData + Offset, sizeof(size_t));

		CString addressText;
		addressText.Format("%#x", Address);
		m_AddressInfoList.InsertItem(i, addressText);       //将得到的数据加入到列表当中
		CString valueText;
		valueText.Format("%d", m_TargetValue);
		m_AddressInfoList.SetItemText(i, 1, valueText);

		//跳过这个数据结构进入下一个循环
		Offset += sizeof(DWORD);

	}

}
void CChangeMemoryDlg::OnBnClickedOkButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString TargetValueStr;
	m_edit2.GetWindowText(TargetValueStr);
	m_TargetValue = _ttoi(TargetValueStr);

	int bufferLength = sizeof(BYTE) + sizeof(int);
	LPBYTE bufferData = new BYTE[bufferLength];
	bufferData[0] = CLIENT_FIRST_SCAN_REQUIRE;
	memcpy(bufferData + sizeof(BYTE), &m_TargetValue, sizeof(int));
	m_IocpServer->OnPrepareSending(m_ContextObject, bufferData, bufferLength);
}


void CChangeMemoryDlg::OnBnClickedScanAgainButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString TargetValueStr;
	m_edit2.GetWindowText(TargetValueStr);
	m_TargetValue = _ttoi(TargetValueStr);

	int bufferLength = sizeof(BYTE) + sizeof(int);
	LPBYTE bufferData = new BYTE[bufferLength];
	bufferData[0] = CLIENT_SCAN_AGAIN_REQUIRE;
	memcpy(bufferData + sizeof(BYTE), &m_TargetValue, sizeof(int));
	m_IocpServer->OnPrepareSending(m_ContextObject, bufferData, bufferLength);
	
}



void CChangeMemoryDlg::OnNMDblclkTargetAddressList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//将所选地址保存到全局
	CListCtrl* listCtrl = &m_AddressInfoList;
	POSITION position = listCtrl->GetFirstSelectedItemPosition();
	int	item = listCtrl->GetNextSelectedItem(position);
	CString itemText = listCtrl->GetItemText(item, 0);
	 __TargetAddress = _tcstoul(itemText, NULL, 0);	//将字符串转换为数字
	CEditValueDlg* Dialog = new CEditValueDlg(this, &itemText,m_IocpServer, m_ContextObject);
	
	
	// 设置父窗口为卓面
	Dialog->Create(IDD_CHANGE_VALUE_DIALOG, GetDesktopWindow());    //创建非阻塞的Dlg
	Dialog->ShowWindow(SW_SHOW);

	*pResult = 0;

}
