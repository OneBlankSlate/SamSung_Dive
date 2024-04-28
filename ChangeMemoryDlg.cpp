
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


/*****Client��ͷ*****/
COLUMN_DATA __AddressInfoList[] =
{
	{ "Address",	    240 },
	{ "Value",			200 },
};
// CChangeMemoryDlg �Ի���

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


// CChangeMemoryDlg ��Ϣ�������


BOOL CChangeMemoryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	
	TCHAR v1[10]={0};
	//memcpy(v1, __ProcessIdentity, sizeof(HANDLE)); 

	_stprintf(v1, _T("%u"), (DWORD)__ProcessIdentity);

	/*char* v1 = 0;
	itoa((int)__ProcessIdentity, v1, 10);*/
	m_edit1.SetWindowText(v1);

	for (int i = 0; i < sizeof(__AddressInfoList) / sizeof(COLUMN_DATA); i++)
	{
		//����ClientInfoList�ı�����
		m_AddressInfoList.InsertColumn(i, __AddressInfoList[i].TitleData,
			LVCFMT_CENTER, __AddressInfoList[i].TitleWidth);
		//LVCFMT_CENTER�����־���
	}
	m_AddressInfoList.SetExtendedStyle(LVS_EX_FULLROWSELECT);


	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CChangeMemoryDlg::OnEnChangeProcessIdentity()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

void CChangeMemoryDlg::ShowAddressInfoList()
{
	
	BYTE* BufferData = (BYTE*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));

	DWORD	Offset = 0;
	size_t v1;
	m_AddressInfoList.DeleteAllItems();
	//������������ÿһ���ַ��������������ݽṹ�� Id+������+0+������+0
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		size_t Address;
		memcpy(&Address, BufferData + Offset, sizeof(size_t));

		CString addressText;
		addressText.Format("%#x", Address);
		m_AddressInfoList.InsertItem(i, addressText);       //���õ������ݼ��뵽�б���
		CString valueText;
		valueText.Format("%d", m_TargetValue);
		m_AddressInfoList.SetItemText(i, 1, valueText);

		//����������ݽṹ������һ��ѭ��
		Offset += sizeof(DWORD);

	}

}
void CChangeMemoryDlg::OnBnClickedOkButton()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//����ѡ��ַ���浽ȫ��
	CListCtrl* listCtrl = &m_AddressInfoList;
	POSITION position = listCtrl->GetFirstSelectedItemPosition();
	int	item = listCtrl->GetNextSelectedItem(position);
	CString itemText = listCtrl->GetItemText(item, 0);
	 __TargetAddress = _tcstoul(itemText, NULL, 0);	//���ַ���ת��Ϊ����
	CEditValueDlg* Dialog = new CEditValueDlg(this, &itemText,m_IocpServer, m_ContextObject);
	
	
	// ���ø�����Ϊ׿��
	Dialog->Create(IDD_CHANGE_VALUE_DIALOG, GetDesktopWindow());    //������������Dlg
	Dialog->ShowWindow(SW_SHOW);

	*pResult = 0;

}
