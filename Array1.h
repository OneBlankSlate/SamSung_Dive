#pragma once
#include <windows.h>
#include <iostream>
#include <math.h>
using namespace std;
#pragma comment(lib,"zlib.lib")
class CArray1
{
public:
	CArray1();
	~CArray1();
	BOOL WriteArray(PUINT8 BufferData, ULONG_PTR BufferLength);  //HelloWorld  10
	ULONG_PTR ReallocateArray(ULONG_PTR BufferLength);
	ULONG_PTR GetArrayMaximumLength();
	ULONG_PTR GetArrayLength();
	PUINT8 GetArray(ULONG_PTR Position=0);
	VOID ClearArray();
	ULONG_PTR DeallocateArray(ULONG_PTR BufferLength);
	ULONG_PTR ReadArray(PUINT8 BufferData, ULONG_PTR BufferLength);
	ULONG_PTR RemoveArray(ULONG_PTR BufferLength);

private:
	PUINT8	    m_BufferData;          //������ָ��
	PUINT8	    m_CheckPosition;
	ULONG_PTR	m_MaximumLength;       //�ڴ��С
	CRITICAL_SECTION  m_CriticalSection;
};
