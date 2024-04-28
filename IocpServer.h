#pragma once
#include<Windows.h>
#include<iostream>
#include<winsock2.h>    //ͨ���׽���ͷ�ļ�
#include"Array1.h"
#include"CriticalSection1.h"
#include<mstcpip.h>        //�������ͷ�ļ�
#include"zlib.h"
#include"zconf.h"
#define WORK_THREAD_MAX 2
#define PACKET_FLAG_LENGTH 5
#define PACKET_HEADER_LENGTH 13
using namespace std;
#pragma comment(lib,"Ws2_32.lib")   //�ó������"Ws2_32.lib"��  ���������еĵ�������(ͨ�Žӿڣ�
#define PACKET_LENGTH 0x2000



enum PACKET_TYPE
{
	IO_INITIALIZE,
	IO_RECEIVE,
	IO_SEND,
	IO_IDLE
};

class COVERLAPPEDEX
{
public:

	OVERLAPPED			m_Overlapped;   //�������ص��ṹ  ��ǰ�첽IO����õ����ʱ���ǵĴ����ܹ���Ӧ
	PACKET_TYPE			m_PackType;     //ö��  4  4 

	COVERLAPPEDEX(PACKET_TYPE PackType)
	{
		ZeroMemory(this, sizeof(COVERLAPPEDEX));
		m_PackType = PackType;
	}
};

typedef struct _CONTEXT_OBJECT_
{
	SOCKET ClientSocket;    //Serverʹ�ø��׽�����Ŀ��Client����ͨ��
	WSABUF ReceiveBuffer;   //�����û����������  �ýṹ����û���ڴ棬��m_BufferData����
	WSABUF SendBuffer;      //��ͻ��������ݣ��ó�Ա��m_SendCompressedBufferData������
	char BufferData[PACKET_LENGTH];

	//ѹ���㷨
	CArray1 m_ReceivedCompressedBufferData;      //���յ���ѹ������
	CArray1 m_ReceivedDecompressedBufferData;    //��ѹ֮�������
	CArray1 m_SendCompressedBufferData;         //��Ҫ���͵�ѹ������

	VOID InitializeMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(BufferData, 0, sizeof(char) * PACKET_LENGTH);
		memset(&ReceiveBuffer, 0, sizeof(WSABUF));
		memset(&SendBuffer, 0, sizeof(WSABUF));
	}


	//�ж��������������Ա�����ݣ�˵���Ƕ�������
	int DlgIdentity;
	HANDLE DlgHandle;    //HWND(�û�����)     HANDLE(�ں˶���)
	HANDLE    ChangeMemoryDlg;   //MemoryEditor����
	HANDLE    VMMapDlg;

} CONTEXT_OBJECT, * PCONTEXT_OBJECT;
typedef CList<PCONTEXT_OBJECT> 	CONTEXT_LIST;      //MFC�Ĺٷ���ģ��

typedef void (CALLBACK* LPFN_WNDCALLBACK)(PCONTEXT_OBJECT ContextObject);  //����ָ�붨�� 

class CIocpServer
{
public:
	CIocpServer();
	~CIocpServer();
	BOOL ServerRun(USHORT ListenPort, LPFN_WNDCALLBACK WndCallback);
	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData);    //�̻߳ص������ĵڶ��ַ�������ľ�̬��Ա����
	void InitIocp();
	void OnAccept();
	PCONTEXT_OBJECT AllocateContextObject();
	VOID RemoveContextObject(CONTEXT_OBJECT* ContextObject, LPOVERLAPPED Overlapped);
	BOOL HandleIo(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject,
		DWORD NumberOfBytesTransferred, LPOVERLAPPED Overlapped);
	VOID PostReceive(CONTEXT_OBJECT* ContextObject);           //�첽IO
	VOID MoveContextObjectToFreePool(CONTEXT_OBJECT* ContextObject);
	BOOL OnReceiving(PCONTEXT_OBJECT  ContextObject, DWORD BufferLength, LPOVERLAPPED Overlapped);  //�����м���overlapped
	VOID OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength);
	BOOL OnSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength, LPOVERLAPPED Overlapped);

public:
	LPFN_WNDCALLBACK m_WndCallback;
	SOCKET m_ListenSocket;   //�����׽���
	HANDLE m_ListenThreadHandle;  //�����߳�
	ULONG m_ThreadPoolMin;
	ULONG m_ThreadPoolMax;
	HANDLE m_WorkThreadHandle[WORK_THREAD_MAX];
	ULONG m_WorkThreadCount;
	BOOL m_IsWorking=TRUE;
	HANDLE m_KillEventHandle;   //�����¼��������ţ�������Դ
	HANDLE m_IocpServer;
	HANDLE m_ListenEventHandle;
	volatile long m_CurrentThreadCount;
	volatile long m_BusyThreadCount;

	//�ٽ������� ��������±����Ķ�����в�������
	CRITICAL_SECTION m_CriticalSection;
	CONTEXT_LIST  m_FreeContextList;    //�ڴ��ģ��
	CONTEXT_LIST  m_ConnectContextList;   //�����û����±���������
	
	HANDLE m_CompletionPortHandle;//������ɶ˿�
	ULONG m_KeepAliveTime;
	HANDLE m_PacketHeaderFlag[WORK_THREAD_MAX];
private:
	
};
DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData);




