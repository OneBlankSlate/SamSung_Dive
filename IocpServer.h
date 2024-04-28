#pragma once
#include<Windows.h>
#include<iostream>
#include<winsock2.h>    //通信套接字头文件
#include"Array1.h"
#include"CriticalSection1.h"
#include<mstcpip.h>        //保活机制头文件
#include"zlib.h"
#include"zconf.h"
#define WORK_THREAD_MAX 2
#define PACKET_FLAG_LENGTH 5
#define PACKET_HEADER_LENGTH 13
using namespace std;
#pragma comment(lib,"Ws2_32.lib")   //让程序加载"Ws2_32.lib"库  以利用其中的导出函数(通信接口）
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

	OVERLAPPED			m_Overlapped;   //真正的重叠结构  当前异步IO请求得到完成时我们的代码能够响应
	PACKET_TYPE			m_PackType;     //枚举  4  4 

	COVERLAPPEDEX(PACKET_TYPE PackType)
	{
		ZeroMemory(this, sizeof(COVERLAPPEDEX));
		m_PackType = PackType;
	}
};

typedef struct _CONTEXT_OBJECT_
{
	SOCKET ClientSocket;    //Server使用该套接字与目标Client进行通信
	WSABUF ReceiveBuffer;   //接收用户到达的数据  该结构本身没有内存，与m_BufferData关联
	WSABUF SendBuffer;      //向客户发送数据（该成员与m_SendCompressedBufferData关联）
	char BufferData[PACKET_LENGTH];

	//压缩算法
	CArray1 m_ReceivedCompressedBufferData;      //接收到的压缩数据
	CArray1 m_ReceivedDecompressedBufferData;    //解压之后的数据
	CArray1 m_SendCompressedBufferData;         //将要发送的压缩数据

	VOID InitializeMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(BufferData, 0, sizeof(char) * PACKET_LENGTH);
		memset(&ReceiveBuffer, 0, sizeof(WSABUF));
		memset(&SendBuffer, 0, sizeof(WSABUF));
	}


	//判断如果下面两个成员有数据，说明是二级窗口
	int DlgIdentity;
	HANDLE DlgHandle;    //HWND(用户对象)     HANDLE(内核对象)
	HANDLE    ChangeMemoryDlg;   //MemoryEditor窗口
	HANDLE    VMMapDlg;

} CONTEXT_OBJECT, * PCONTEXT_OBJECT;
typedef CList<PCONTEXT_OBJECT> 	CONTEXT_LIST;      //MFC的官方类模板

typedef void (CALLBACK* LPFN_WNDCALLBACK)(PCONTEXT_OBJECT ContextObject);  //函数指针定义 

class CIocpServer
{
public:
	CIocpServer();
	~CIocpServer();
	BOOL ServerRun(USHORT ListenPort, LPFN_WNDCALLBACK WndCallback);
	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData);    //线程回调函数的第二种方法，类的静态成员函数
	void InitIocp();
	void OnAccept();
	PCONTEXT_OBJECT AllocateContextObject();
	VOID RemoveContextObject(CONTEXT_OBJECT* ContextObject, LPOVERLAPPED Overlapped);
	BOOL HandleIo(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject,
		DWORD NumberOfBytesTransferred, LPOVERLAPPED Overlapped);
	VOID PostReceive(CONTEXT_OBJECT* ContextObject);           //异步IO
	VOID MoveContextObjectToFreePool(CONTEXT_OBJECT* ContextObject);
	BOOL OnReceiving(PCONTEXT_OBJECT  ContextObject, DWORD BufferLength, LPOVERLAPPED Overlapped);  //参数中加入overlapped
	VOID OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength);
	BOOL OnSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength, LPOVERLAPPED Overlapped);

public:
	LPFN_WNDCALLBACK m_WndCallback;
	SOCKET m_ListenSocket;   //监听套接字
	HANDLE m_ListenThreadHandle;  //监听线程
	ULONG m_ThreadPoolMin;
	ULONG m_ThreadPoolMax;
	HANDLE m_WorkThreadHandle[WORK_THREAD_MAX];
	ULONG m_WorkThreadCount;
	BOOL m_IsWorking=TRUE;
	HANDLE m_KillEventHandle;   //当该事件对象授信，销毁资源
	HANDLE m_IocpServer;
	HANDLE m_ListenEventHandle;
	volatile long m_CurrentThreadCount;
	volatile long m_BusyThreadCount;

	//临界区对象 负责对上下背景文对象进行操作保护
	CRITICAL_SECTION m_CriticalSection;
	CONTEXT_LIST  m_FreeContextList;    //内存池模板
	CONTEXT_LIST  m_ConnectContextList;   //上线用户上下背景文链表
	
	HANDLE m_CompletionPortHandle;//定义完成端口
	ULONG m_KeepAliveTime;
	HANDLE m_PacketHeaderFlag[WORK_THREAD_MAX];
private:
	
};
DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData);




