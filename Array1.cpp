#include "pch.h"
#include "Array1.h"
#define U_PAGE_ALIGNMENT 3
#define F_PAGE_ALIGNMENT 3.0
CArray1::CArray1()
{
	m_MaximumLength = 0;
	m_BufferData = m_CheckPosition = NULL;
	InitializeCriticalSection(&m_CriticalSection);   //�ٽ���or�ؼ���    �߳�ͬ��
}

CArray1::~CArray1()
{
	if (m_BufferData)
	{
		VirtualFree(m_BufferData, 0, MEM_RELEASE);   //���������ַ�ռ�   VirtualAlloc VirtualFree  new delete
		m_BufferData = NULL;
	}

	DeleteCriticalSection(&m_CriticalSection);
	m_BufferData = m_CheckPosition = NULL;
	m_MaximumLength = 0;
}
BOOL CArray1::WriteArray(PUINT8 BufferData, ULONG_PTR BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);

	//Array[][][][][][][][][][][][][] HelloWorld    xx

	// 
	//[5][4][3][2][1] 1 1 1 1   [][] [] [] 

	//[][][][][][][]  [] [] [] [] 

	if (ReallocateArray(BufferLength + GetArrayLength()) == (ULONG_PTR)-1)   //�ڴ�ռ䲻��
	{
		LeaveCriticalSection(&m_CriticalSection);  //����ʧ��
		return FALSE;
	}

	CopyMemory(m_CheckPosition, BufferData, BufferLength);

	m_CheckPosition += BufferLength;
	LeaveCriticalSection(&m_CriticalSection);
	return TRUE;
}
ULONG_PTR CArray1::ReallocateArray(ULONG_PTR BufferLength)   //���bufferLength�����������ڴ�ʱ���ܴ�С�����WriteArray�еĵ���
{
	if (BufferLength < GetArrayMaximumLength())   //��ǰ���ݴ�С С�ڵ�ǰ��������ڴ��ܴ�С�����������������ڴ�
		return 0;

	//����ȡ�߽�   11/3.0 = 
	ULONG_PTR  v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;
	PUINT8  v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (v5 == NULL)
	{
		return -1;
	}
	//ԭ�ȵ���Ч���ݳ���  
	ULONG_PTR v3 = GetArrayLength();
	//����ԭ�����ݵ��µ�������ڴ���
	CopyMemory(v5, m_BufferData, v3);

	//�ͷ�ԭ���ڴ�
	if (m_BufferData)
	{
		VirtualFree(m_BufferData, 0, MEM_RELEASE);
	}
	m_BufferData = v5;
	m_CheckPosition = m_BufferData + v3;   //m_CheckPositionָ���ڴ���д�

	m_MaximumLength = v7;

	return m_MaximumLength;
}

ULONG_PTR CArray1::GetArrayMaximumLength()  //�ܹ��ڴ��С
{
	return m_MaximumLength;
}
ULONG_PTR CArray1::GetArrayLength()        //��Ч������ռ�ڴ��С
{
	if (m_BufferData == NULL)
		return 0;

	return (ULONG_PTR)m_CheckPosition - (ULONG_PTR)m_BufferData;

}
PUINT8 CArray1::GetArray(ULONG_PTR Position)  //HelloWorld
{
	if (m_BufferData == NULL)
	{
		return NULL;
	}

	if (Position >= GetArrayLength())
	{
		return NULL;
	}
	return m_BufferData + Position;
}
VOID CArray1::ClearArray()
{
	EnterCriticalSection(&m_CriticalSection);
	m_CheckPosition = m_BufferData;
	//ֻ����1024�ڴ�
	DeallocateArray(1024);  //�����ڴ�
	LeaveCriticalSection(&m_CriticalSection);
}
ULONG_PTR CArray1::DeallocateArray(ULONG_PTR BufferLength)
{
	//4096�ڴ�ռ�
	//����1024����Ч����
	if (BufferLength < GetArrayLength())  //HelloWorld[][][][][][][][][][][][][][][][][][][][][][][][][][][]
		return 0;

	//��Ч����С��1024  ֻ����1024���ڴ�ռ�
	ULONG_PTR v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;

	if (GetArrayMaximumLength() <= v7)
	{
		return 0;
	}
	//���������µ�1024���ڴ�
	PUINT8 v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT, PAGE_READWRITE);

	ULONG_PTR v3 = GetArrayLength();  //��ԭ���ڴ����Ч����   ����ĳ�
	CopyMemory(v5, m_BufferData, v3);

	VirtualFree(m_BufferData, 0, MEM_RELEASE);

	m_BufferData = v5;
	m_CheckPosition = m_BufferData + v3;
	m_MaximumLength = v7;
	return m_MaximumLength;
}
ULONG_PTR CArray1::ReadArray(PUINT8 BufferData, ULONG_PTR BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);    //�����ٽ�������ֹ���̸߳���
	if (BufferLength > GetArrayMaximumLength())  //�ڴ�ռ���ܳ�   �ж��ٸ�����
	{
		LeaveCriticalSection(&m_CriticalSection);
		return 0;
	}
	if (BufferLength > GetArrayLength())   //400����  300��   350�� 
	{
		BufferLength = GetArrayLength();   //300��
	}

	if (BufferLength)
	{
		CopyMemory(BufferData, m_BufferData, BufferLength);//��Arry���ݿ�����������
		//����ǰ�Ƹ���
		MoveMemory(m_BufferData, m_BufferData + BufferLength, GetArrayMaximumLength() - BufferLength);
		m_CheckPosition -= BufferLength;
	}
	DeallocateArray(GetArrayLength());//���ճ��� 
	LeaveCriticalSection(&m_CriticalSection);
	return BufferLength;
}

ULONG_PTR CArray1::RemoveArray(ULONG_PTR BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);
	if (BufferLength > GetArrayMaximumLength())
	{
		return 0;
	}
	if (BufferLength > GetArrayLength())
	{
		BufferLength = GetArrayLength();
	}
	if (BufferLength)
	{
		//�������ﳵǰ�Ƹ���
		MoveMemory(m_BufferData, m_BufferData + BufferLength, GetArrayMaximumLength() - BufferLength);   //����ǰ��  [Shinexxxx??]
		m_CheckPosition -= BufferLength;
	}
	DeallocateArray(GetArrayLength());
	LeaveCriticalSection(&m_CriticalSection);
	return BufferLength;
}
