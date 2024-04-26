#include "pch.h"
#include "IocpServer.h"
#include"CriticalSection1.h"
CIocpServer::CIocpServer()
{
	//套接字类库的初始化
	WSADATA v1 = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &v1)!=0)   //创建成功返回值为0
	{
		//初始化失败
	}
	m_ListenSocket = INVALID_SOCKET;
	m_ListenThreadHandle = INVALID_HANDLE_VALUE;
	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;  //控制线程池调度
	m_WorkThreadCount = 0;
	m_IsWorking = TRUE;   //控制工作线程循环
	m_KillEventHandle = NULL;  
	m_IocpServer = NULL;
	for (int i = 0; i < WORK_THREAD_MAX; i++)
	{
		m_WorkThreadHandle[i] = INVALID_HANDLE_VALUE;
	}

	m_ListenEventHandle = NULL;
	InitializeCriticalSection(&m_CriticalSection);
	m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	m_CurrentThreadCount = 0;
	m_BusyThreadCount = 0;
	memcpy(m_PacketHeaderFlag, "Shine", PACKET_FLAG_LENGTH);
}

CIocpServer::~CIocpServer()
{
	//回收资源
	Sleep(1);   //主线程停顿一下，给子线程一个执行的机会

	SetEvent(m_KillEventHandle);    //触发事件使其正常退出监听线程的循环

	WaitForSingleObject(m_ListenThreadHandle, INFINITE);   //等待监听线程退出
	CloseHandle(m_ListenThreadHandle);
	m_ListenThreadHandle = INVALID_HANDLE_VALUE;


	if (m_ListenSocket != INVALID_SOCKET)                //监听套接字的关闭
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}

	if (m_ListenEventHandle != NULL)                //监听事件的关闭
	{
		CloseHandle(m_ListenEventHandle);
		m_ListenSocket = NULL;
	}
	if (m_KillEventHandle != NULL)                //m_KillEventHandle事件的关闭
	{
		CloseHandle(m_KillEventHandle);
		m_KillEventHandle = NULL;
	}
	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;
	m_WorkThreadCount = 0;
	m_IsWorking = FALSE;                //工作线程退出

	if (m_CompletionPortHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_CompletionPortHandle);
		m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	}
	WaitForMultipleObjects(WORK_THREAD_MAX, m_WorkThreadHandle, TRUE, INFINITE);
	for (int i = 0; i < WORK_THREAD_MAX; i++)
	{
		CloseHandle(m_WorkThreadHandle[i]);
		m_WorkThreadHandle[i] = INVALID_HANDLE_VALUE;
	}

	DeleteCriticalSection(&m_CriticalSection);

	
	WSACleanup();   //套接字资源的回收






}

BOOL CIocpServer::ServerRun(USHORT ListenPort, LPFN_WNDCALLBACK WndCallback)
{

	BOOL IsOk = TRUE;
	SOCKADDR_IN	ServerAddress;   //结构 

	m_WndCallback = WndCallback;
	//两种创建事件的方法
	//创建事件对象(kernel32.dll)
	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);   //匿名事件

	if (m_KillEventHandle == NULL)
	{
		return FALSE;
	}

	//创建监听套接字

	//应用   Http  
	//表示
	//会话
	//传输   TCP(Stream)  UDP(datagram)   
	//网络   IP  寻址
	//数据链路
	//物理
	//创建一个监听套接字
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);   //Address family internet
	if (m_ListenSocket == INVALID_SOCKET)			  //-1									 //重叠，让创建出来的保安有异步I/O的能力
	{				
		IsOk = FALSE;    
		goto Error;
	}

	//创建一个监听事件(网络代码)(Ws2_32.dll)  该库需要在头文件中自己加载   #pragma comment(lib,"Ws2_32.lib") 
	m_ListenEventHandle = WSACreateEvent();    //WSACreateEvent 函数创建初始状态为“未签名”的手动重置事件对象。
	//等效于CreateEvent(NULL, TRUE, FALSE, NULL)
		 
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		goto Error;
	}

	//将监听套接字与监听事件进行关联，并授予FD_ACCEPT与FD_CLOSE属性
	IsOk = WSAEventSelect(m_ListenSocket, m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);  //让监听事件对这两种网络事件敏感
	if (IsOk == SOCKET_ERROR)  //如果应用程序的网络事件和关联的事件对象的规范成功，则返回值为零。
							   //否则，将返回值 SOCKET_ERROR，并且可以通过调用 WSAGetLastError 来检索特定的错误号。
	{
		goto Error;
	}
	//通信端口 0 - 2^16-1
	//初始化Server端网卡（本地网卡）                        //htons是一种强制类型转换，host to net string
	ServerAddress.sin_port = htons(ListenPort);                    //ini文件设置的监听端口放进结构中
	ServerAddress.sin_family = AF_INET;
	//ServerAddress.sin_addr.s_addr = INADDR_ANY;
	ServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;  //设置当前本地任意网卡的IP地址    

	//绑定套接字
	//套接字与网卡关联
	IsOk = bind(m_ListenSocket,
		(sockaddr*)&ServerAddress,    //函数参数与定义结构不匹配（新结构老函数）
		sizeof(ServerAddress));

	if (IsOk == SOCKET_ERROR)
	{
		int LastError = WSAGetLastError();		 //反复调试反复绑定会出现错误，重启计算机解决
		goto Error;             
	}

	//保安上班监听
	IsOk = listen(m_ListenSocket, SOMAXCONN);   // 0x7fffffff监听能力（达不到也无法测出）  监测并发的能力
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}

	//定义接收线程对象的返回值（线程句柄）
	//创建监听线程    注意线程回调函数的两种定义方式，两种方式都要传入this指针，想要访问类成员的话，都必须要传入this指针
	m_ListenThreadHandle = CreateThread(NULL, 0, ListenThreadProcedure, (void*)this, 0, 0);
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		IsOk = FALSE;
		goto Error;
	}

	//初始化Iocp
	InitIocp();           //创建两个工作线程

	return TRUE;

Error:;

	if (IsOk !=TRUE)
	{
		if (m_ListenSocket != INVALID_SOCKET)
		{
			closesocket(m_ListenSocket);
			m_ListenSocket = INVALID_SOCKET;
		}
	}
	return 0;
}

DWORD WINAPI CIocpServer::ListenThreadProcedure(LPVOID ParameterData)   //类静态成员函数 监听线程的实现
{
	CIocpServer* v1 = (CIocpServer*)ParameterData;
	DWORD    v2 = 0;
	WSANETWORKEVENTS NetWorkEvents;

	while (1)
	{
		v2 = WaitForSingleObject(v1->m_KillEventHandle, 100);
		if (v2 == WAIT_OBJECT_0)    
		{
			//由于析构函数触发m_KillEventHandle事件，在析构中调用了SetEvent()导致的事件授信
			break;
		}

		//等待监听事件授信
		v2 = WSAWaitForMultipleEvents(1, &(v1->m_ListenEventHandle), FALSE, 100, FALSE);  //该事件授信 不是因为SetEvent，是因为网络事件
		if (v2 == WSA_WAIT_TIMEOUT)														  //Accept事件 客户端连接服务器
		{																				  //Close事件  客户端断开连接
			//该事件没有授信
			continue;
		}
		//发生了FD_ACCEPT或者FD_CLOSE事件
		//如果事件授信我们就将该事件转换成一个网络事件进行判断
		v2 = WSAEnumNetworkEvents(v1->m_ListenSocket,           //事件选择模型
			v1->m_ListenEventHandle,
			&NetWorkEvents);   //判断的结果存储在NetWorkEvents变量中

		//判断网路事件
		if (v2 == SOCKET_ERROR)
		{
			//日志处理
			break;
		}
		if (NetWorkEvents.lNetworkEvents & FD_ACCEPT)   //监听套接字授信
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{

				//处理客户端上线请求
				v1->OnAccept();
			}
			else
			{
				break;
			}

		}
		else
		{
			//家长离开

			//当删除一个用户时代码会执行到这里  不要在这里退出循环
			//break;
		}



	}
    //MessageBox(NULL,_T("ListenThreadProcedure"),NULL,NULL);
	return 0;

}

//异步请求完成后的数据分析
DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData)   //没有使用该方法
{
	CIocpServer* v1 = (CIocpServer*)ParameterData;
	HANDLE   CompletionPortHandle = v1->m_CompletionPortHandle;
	DWORD    NumberOfBytesTransferred = 0;
	LPOVERLAPPED     Overlapped = NULL;
	PCONTEXT_OBJECT  ContextObject = NULL;  //一维指针
	COVERLAPPEDEX* OverlappedEx = NULL;     //指针



	//原子锁
	InterlockedIncrement(&v1->m_CurrentThreadCount);  //1  2
	InterlockedIncrement(&v1->m_BusyThreadCount);     //1  2

	ULONG            v3 = 0;
	BOOL             IsOk1 = FALSE;

	while (v1->m_IsWorking==TRUE)
	{


		InterlockedDecrement(&v1->m_BusyThreadCount); // 2   0


		//等待完成端口中的完成数据
			//该函数是阻塞函数如果该函数返回说明有请求得到了完成
		BOOL IsOk2 = GetQueuedCompletionStatus(
			CompletionPortHandle,
			&NumberOfBytesTransferred,                    //完成多少数据
			(PULONG_PTR)&ContextObject,       //完成Key   获得的完成Key就是咱们投递过去的  二维指针
			&Overlapped, 60000);


		DWORD LastError = GetLastError();
		OverlappedEx = CONTAINING_RECORD(Overlapped, COVERLAPPEDEX, m_Overlapped);  //数据强制类型转换


		//Current 2
		//Busy 0

		//m_BusyThreadCount = 1
		//v3 = 1

		//m_BusyThreadCount = 2
		//v3 = 2
		v3 = InterlockedIncrement(&v1->m_BusyThreadCount);

		//完成端口异常(强制退出)
		if (!IsOk2 && LastError != WAIT_TIMEOUT)
		{
			//关闭线程
			if (ContextObject && v1->m_IsWorking == FALSE && NumberOfBytesTransferred == 0)
			{
				//当对方的套机制发生了关闭	
				v1->RemoveContextObject(ContextObject, &OverlappedEx->m_Overlapped);
			}
			continue;
		}



		//线程池调度
		if (!IsOk1)
		{

			//如果你在干活就判断一下是否可以再调度一个线程

			//分配一个新的线程到线程到线程池
			if (v3 == v1->m_CurrentThreadCount)
			{

				if (v3 < v1->m_ThreadPoolMax)//
				{


					if (ContextObject != NULL)   //你是否在干活
					{

						HANDLE ThreadHandle = (HANDLE)CreateThread(NULL,
							0,
							(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
							(void*)v1,
							0,
							NULL);
						InterlockedIncrement(&v1->m_WorkThreadCount);

						CloseHandle(ThreadHandle);
					}


				}
			}
			//销毁一个线程从线程池
			if (!IsOk2 && LastError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					{
						if (v1->m_CurrentThreadCount > v1->m_ThreadPoolMin)
						{
							break;   //销毁一个线程
						}
					}
					IsOk2 = TRUE;
				}
			}
		}

		//处理完成的请求
		if (!IsOk1)
		{
			//当前线程要对完成请求进行处理
			//请求得到完成(两种请求 IO_INITIALIZE IO_RECEIVE )
			if (IsOk2 && OverlappedEx != NULL && ContextObject != NULL)  //完成端口正常完成  事件不为空   请求对象不为空
			{
				try
				{

					//请求得到完成的处理函数
					v1->HandleIo(OverlappedEx->m_PackType, ContextObject, NumberOfBytesTransferred, Overlapped);

					//没有释放内存
					ContextObject = NULL;

				}
				catch (...) {}
			}
		}


		if (OverlappedEx)
		{
			delete OverlappedEx;
			OverlappedEx = NULL;
		}


	}

	InterlockedDecrement(&v1->m_WorkThreadCount);
	InterlockedDecrement(&v1->m_CurrentThreadCount);
	InterlockedDecrement(&v1->m_BusyThreadCount);


	TCHAR v10[0x1000] = { 0 };
	_stprintf_s(v10, _T("ThreadIdentity:%d"), GetCurrentThreadId());
	//MessageBox(NULL, _T("WorkThreadProcedure"), v10, 0);

	return 0;

}

BOOL CIocpServer::HandleIo(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject,
	DWORD NumberOfBytesTransferred, LPOVERLAPPED Overlapped)
{
	BOOL v1 = FALSE;

	if (IO_INITIALIZE == PacketType)
	{
		//MessageBox(NULL, _T("IO_INITIALIZE"), _T("IO_INITIALIZE"), 0);
	}

	if (IO_RECEIVE == PacketType)
	{
		//完成SendLoginInformation数据分析
		v1 = OnReceiving(ContextObject, NumberOfBytesTransferred,Overlapped);
	}

	if (IO_SEND == PacketType)
	{
		v1 = OnSending(ContextObject, NumberOfBytesTransferred, Overlapped);
	}
	return v1;
}


BOOL CIocpServer::OnReceiving(PCONTEXT_OBJECT  ContextObject, DWORD BufferLength, LPOVERLAPPED Overlapped)
{
	CCriticalSection1 CriticalSection(&m_CriticalSection);
	try
	{

		if (BufferLength == 0)
		{
			//对方关闭了套接字
			//MessageBox(NULL, _T("关闭套接字"), _T("关闭套接字"), 0);
			RemoveContextObject(ContextObject,Overlapped);
			return FALSE;
		}


		//将接到的数据拷贝到m_ReceivedCompressedBufferData
		ContextObject->m_ReceivedCompressedBufferData.WriteArray(
			(PBYTE)ContextObject->BufferData, BufferLength);
		//将接收到的数据拷贝到我们自己的内存中wsabuff    8192

//读取数据包的头部(数据包的头部是不参与压缩的)
		while (ContextObject->m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			//存储数据包头部标志
			char v1[PACKET_FLAG_LENGTH] = { 0 };//Shine[][][]


			//拷贝数据包头部标志
			CopyMemory(v1, ContextObject->m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);


			//校验数据包头部标志
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw "Bad Buffer";
			}

			//获取数据包总大小
			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength,
				ContextObject->m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH),
				sizeof(ULONG));


			if (PackTotalLength &&
				(ContextObject->m_ReceivedCompressedBufferData.GetArrayLength()) >= PackTotalLength)
			{
				//[Shine][压缩的长度+13][没有压缩的长度][HelloWorld.......]
				ULONG DecompressedLength = 0;

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);

				//[压缩的长度+13][没有压缩的长度][HelloWorld.......]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength,
					sizeof(ULONG));

				//[没有压缩的长度][HelloWorld.......]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength,
					sizeof(ULONG));

				//[HelloWorld.......]
				ULONG CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;   //被压缩后的真实数据的长度

				//压缩数据
				PBYTE CompressedData = new BYTE[CompressedLength];

				//解压缩数据
				PBYTE DecompressedData = new BYTE[DecompressedLength];  //解压过的数据长度 

				if (CompressedData == NULL || DecompressedData == NULL)
				{
					throw "Bad Allocate";

				}

				//从数据包中获取压缩后的数据
				ContextObject->m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);


				//解压缩
				int	IsOk = uncompress(DecompressedData,
					&DecompressedLength, CompressedData, CompressedLength);

				if (IsOk == Z_OK)
				{
					ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
					ContextObject->m_ReceivedCompressedBufferData.ClearArray();

					//拷贝真实数据
					ContextObject->m_ReceivedDecompressedBufferData.WriteArray(DecompressedData,
						DecompressedLength);


					delete[] CompressedData;
					delete[] DecompressedData;

					//窗口回调函数(函数指针)
					m_WndCallback(ContextObject);  //通知窗口  接受到的被解压的数据 
				}
				else
				{
					delete[] CompressedData;
					delete[] DecompressedData;
					throw "Bad Buffer";
				}



			}
			else
			{
				break;
			}
		}

		//上一次的异步请求已经得到完成重新投递新的异步请求
		PostReceive(ContextObject);
	}
	catch (...)
	{
		ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
		ContextObject->m_ReceivedCompressedBufferData.ClearArray();

		PostReceive(ContextObject);
	}

	return TRUE;

}

VOID CIocpServer::OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength)
{
	if (ContextObject == NULL)
	{
		return;   //没有目标
	}

	try
	{
		if (BufferLength > 0)
		{
			unsigned long	CompressedLength = (double)BufferLength * 1.001 + 12;
			LPBYTE			CompressedData = new BYTE[CompressedLength];
			int	IsOk = compress(CompressedData, &CompressedLength, (LPBYTE)BufferData, BufferLength);

			if (IsOk != Z_OK)
			{
				delete[] CompressedData;
				return;
			}
			ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGTH;
			//构建数据包头部
			ContextObject->m_SendCompressedBufferData.WriteArray((LPBYTE)m_PacketHeaderFlag, PACKET_FLAG_LENGTH);
			//[Shine]
			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
			//[Shine][PackTotalLength]
			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
			//[Shine][PackTotalLength][BufferLength]
			ContextObject->m_SendCompressedBufferData.WriteArray(CompressedData, CompressedLength);
			//[Shine][PackTotalLength][BufferLength][.....(真实数据)]
			delete[] CompressedData;
		}
		COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_SEND);

		//将该请求投递到完成端口  
		PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (ULONG_PTR)ContextObject,
			&OverlappedEx->m_Overlapped);
	}
	catch (...) {}
}
BOOL CIocpServer::OnSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength, LPOVERLAPPED Overlapped)
{
	try
	{
		DWORD Flags = MSG_PARTIAL;   //没有意义
		//将完成的数据从数据结构中去除
		ContextObject->m_SendCompressedBufferData.RemoveArray(BufferLength);
		//判断还有多少数据需要发送
		if (ContextObject->m_SendCompressedBufferData.GetArrayLength() == 0)
		{
			//数据已经发送完毕
			ContextObject->m_SendCompressedBufferData.ClearArray();
			return true;
		}
		else
		{
			//真正发送数据
			COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_SEND);

			//将压缩后的数据关联至标准的数据类型中
			ContextObject->SendBuffer.buf = (char*)ContextObject->m_SendCompressedBufferData.GetArray();
			ContextObject->SendBuffer.len = ContextObject->m_SendCompressedBufferData.GetArrayLength();


			//如果该wsa函数真正完成
			int IsOk = WSASend(ContextObject->ClientSocket,
				&ContextObject->SendBuffer,
				1,
				&ContextObject->SendBuffer.len,
				Flags,
				&OverlappedEx->m_Overlapped,    //构建新的异步请求
				NULL);
			if (IsOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				//构建失败了
				RemoveContextObject(ContextObject, Overlapped);   //检查曾经的异步回收
			}
		}
	}
	catch (...) {}
	return FALSE;
}


void CIocpServer::InitIocp()
{

	//创建完成端口
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPortHandle == NULL)
	{
		return;
	}
	//定义一个官方结构体变量
	SYSTEM_INFO SystemInfo;
	//获取系统信息 来得到系统的核数
	GetSystemInfo(&SystemInfo);     

	//线程池
	m_ThreadPoolMin = 1;
	m_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;

	//启动两个工作线程
	ULONG  WorkThreadCount = WORK_THREAD_MAX;
	

	for (int i = 0; i < WorkThreadCount; i++)
	{
		
		m_WorkThreadHandle[i] = CreateThread(NULL,	  
			0,
			(LPTHREAD_START_ROUTINE)WorkThreadProcedure,             
			(void*)this,
			0,
			NULL);  //不关心线程Identity
		if (m_WorkThreadHandle[i] == NULL)
		{
			CloseHandle(m_CompletionPortHandle);
			return;
		}
		m_WorkThreadCount++;
	}
}

void CIocpServer::OnAccept()
{

	//MessageBox(NULL, _T("OnAccept()"), NULL, 0);   

	//客户端上线
	int			Result = 0;

	//保存上线用户IP地址
	SOCKET		ClientSocket = INVALID_SOCKET;    //通信套接字
	SOCKADDR_IN	ClientAddress = { 0 };            //存储客户端地址
	int			ClientAddressLength = sizeof(SOCKADDR_IN);

	//通过我们的监听套接字来生成一个与之信号通信的套接字


	//服务器针对该次客户端链接请求的响应的通信套接字    connect请求的响应
	ClientSocket = accept(m_ListenSocket,
		(sockaddr*)&ClientAddress,   //accept函数对该结构赋值，强转是因为结构体类型的版本较高，函数版本较低
		&ClientAddressLength);


	if (ClientSocket == SOCKET_ERROR)
	{
		return;
	}


	//我们在这里为每一个到达的信号维护了一个与之关联的数据结构这里简称为用户的上下背景文
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();

	if (ContextObject == NULL)
	{

		closesocket(ClientSocket);   //关闭链接
		ClientSocket = INVALID_SOCKET;
		return;
	}


	//成员赋值
	ContextObject->ClientSocket = ClientSocket;  //Send Recv

	//关联内存
	ContextObject->ReceiveBuffer.buf = (char*)ContextObject->BufferData;  
	ContextObject->ReceiveBuffer.len = sizeof(ContextObject->BufferData);

	//将生成的通信套接字与完成端口句柄相关联
	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket,
		m_CompletionPortHandle, (ULONG_PTR)ContextObject, 0);     //第三参数：指针 完成Key
	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;   //销毁对象
		ContextObject = NULL;
		if (ClientSocket != INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
		return;
	}

	//保活机制
	//设置套接字的选项卡 Set KeepAlive 开启保活机制 SO_KEEPALIVE 
	//保持连接检测对方主机是否崩溃如果2小时内在此套接口的任一方向都没
	//有数据交换，TCP就自动给对方 发一个保持存活
	m_KeepAliveTime = 3;     //分钟
	const BOOL IsKeepAlive = TRUE;
	//设置套接字选项  打开机关
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsKeepAlive, sizeof(IsKeepAlive)) != 0)
	{
	}

	//设置超时详细信息
	tcp_keepalive	KeepAlive;
	KeepAlive.onoff = 1; // 启用保活
	KeepAlive.keepalivetime = m_KeepAliveTime;       //超过3分钟没有数据，就发送探测包
	KeepAlive.keepaliveinterval = 1000 * 10;         //重试间隔为10秒 Resend if No-Reply
	WSAIoctl
	(
		ContextObject->ClientSocket,
		SIO_KEEPALIVE_VALS,
		&KeepAlive,
		sizeof(KeepAlive),
		NULL,
		0,
		(unsigned long*)&IsKeepAlive,
		0,
		NULL
	);

	//在做服务器时，如果发生客户端网线或断电等非正常断开的现象，如果服务器没有设置SO_KEEPALIVE选项，
	//则会一直不关闭SOCKET。因为上的设置是默认两个小时时间太长了所以我们就修正这个值

	CCriticalSection1 CriticalSection(&m_CriticalSection);  //Stack Object
	m_ConnectContextList.AddTail(ContextObject);     //插入到我们的内存列表中

	COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_INITIALIZE);
	BOOL IsOk = FALSE;
	//向完成端口中投递一个请求
	//工作线程会等待完成端口的完成状态
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);  //自己向自己的完成端口投递请求

	if ((!IsOk && GetLastError() != ERROR_IO_PENDING))
	{
		//如果投递失败
		RemoveContextObject(ContextObject, &OverlappedEx->m_Overlapped);
		return;
	}

	//该上线用户已经完成了上线的请求
	//服务器向该用户投递PostRecv请求
	PostReceive(ContextObject);   //投递接受客户端数据的异步IO
}

VOID CIocpServer::PostReceive(CONTEXT_OBJECT* ContextObject)
{
	//向我们的刚上线的用户的投递一个接受数据的请求
	//如果该请求得到完成(用户发送数据)
	//工作线程(守候在完成端口)会响应并调用HandleIO函数
	COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_RECEIVE);

	DWORD			ReturnLength;
	ULONG			Flags = MSG_PARTIAL;   //没有意义

	//函数返回但是请求没有得到完成
	int IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->ReceiveBuffer,   //接受数据的内存
		1,
		&ReturnLength,                      //TransferBufferLength
		&Flags,
		&OverlappedEx->m_Overlapped,   //事件
		NULL);
	//返回值是错误 && 但是是错误中的未完成 == 成功
	if (IsOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)   //待定  未完成
	{
		//请求发送错误
		RemoveContextObject(ContextObject, &OverlappedEx->m_Overlapped);   //完犊子  
	}
}

VOID CIocpServer::RemoveContextObject(CONTEXT_OBJECT* ContextObject,LPOVERLAPPED Overlapped)
{

	CCriticalSection1 CriticalSection(&m_CriticalSection);
	//在内存中查找该用户的上下背景文数据结构
	if (m_ConnectContextList.Find(ContextObject))
	{
		//取消在当前套接字的异步IO以前的未完成的异步请求全部立即取消   
		CancelIo((HANDLE)ContextObject->ClientSocket);    //会将该对象上没有得到完成的异步Io立即完成
		//关闭套接字
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		//判断还有没有异步IO请求在当前套接字上
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)Overlapped))   //查看一下完成端口还有没有王浩的箱子   ????????????
		{
			Sleep(1);
		}
		//将该内存结构回收至内存池
		MoveContextObjectToFreePool(ContextObject);   //回收对象内存到内存池
	}
}

VOID CIocpServer::MoveContextObjectToFreePool(CONTEXT_OBJECT* ContextObject)
{

	CCriticalSection1 CriticalSection(&m_CriticalSection);

	POSITION Position = m_ConnectContextList.Find(ContextObject);
	if (Position)
	{

		ContextObject->m_ReceivedCompressedBufferData.ClearArray();
		ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
		ContextObject->m_SendCompressedBufferData.ClearArray();

		memset(ContextObject->BufferData, 0, PACKET_LENGTH);
		m_FreeContextList.AddTail(ContextObject);                         //回收至内存池
		m_ConnectContextList.RemoveAt(Position);                          //从内存结构中移除

	}
}


PCONTEXT_OBJECT CIocpServer::AllocateContextObject()
{
	PCONTEXT_OBJECT ContextObject = NULL;         //分配一个对象

	//进入一个临界区
	CCriticalSection1 CriticalSection(&m_CriticalSection);   //自定义封装了一个线程同步临界区工具
	//判断内存池是否为空
	//内存池
	if (m_FreeContextList.IsEmpty() == FALSE)
	{
		//内存池取内存
		ContextObject = m_FreeContextList.RemoveHead();
	}
	else
	{
		ContextObject = new CONTEXT_OBJECT;   //第一次客户端上下背景文生成的时候
	}
	if (ContextObject != NULL)
	{
		//初始化成员变量
		ContextObject->InitializeMember();  //Clinet BufferData WSABuffer  WSABuffer  Array1 Array2 Array3
	}
	
	return ContextObject;
}

